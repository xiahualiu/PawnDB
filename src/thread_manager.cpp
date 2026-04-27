#include "pawndb/types/thread_manager.h"

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

#include <atomic>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>

#include "pawndb/params.h"
#include "pawndb/schema/demo.h"
#include "pawndb/traits/queue.h"
#include "pawndb/types/buffer_table.h"
#include "pawndb/types/parser.h"
#include "pawndb/types/ret_channel.h"
#include "pawndb/types/worker_table.h"

namespace PawnDB {

ThreadManager::ThreadManager(Database* _db) noexcept
    : db_(*_db), ret_ch_(), workers_(), running_() {}

void ThreadManager::reply(OpAck _ack, buf_parser& _parser,
                          const sockaddr_un& _client_addr,
                          const socklen_t _client_addr_len) noexcept {
  _parser.set_ack(_ack);
  sendto(server_fd_, _parser.get_buffer().data(), _parser.get_buffer_size(), 0,
         reinterpret_cast<const sockaddr*>(&_client_addr), _client_addr_len);
}

void ThreadManager::trait_start() noexcept {
  // Create server socket as UDP domain socket
  int server_fd = socket(AF_UNIX, SOCK_DGRAM, 0);
  if (-1 == server_fd) {
    std::cerr << "Failed to create server socket: " << strerror(errno)
              << std::endl;
    std::exit(EXIT_FAILURE);
  }
  struct sockaddr_un server_addr;
  server_addr.sun_family = AF_UNIX;
  strncpy(server_addr.sun_path, UNIX_SOCKET_PATH,
          sizeof(server_addr.sun_path) - 1);
  errno = 0;

  // Remove dead socket if there is one
  unlink(UNIX_SOCKET_PATH);
  if (errno != 0 && errno != ENOENT) {
    std::cerr << "Failed to unlink the existing socket: " << strerror(errno)
              << std::endl;
    std::exit(EXIT_FAILURE);
  }

  // Bind server socket address
  if (-1 == bind(server_fd, reinterpret_cast<struct sockaddr*>(&server_addr),
                 sizeof(server_addr))) {
    std::cerr << "Failed to bind server socket: " << strerror(errno)
              << std::endl;
    std::exit(EXIT_FAILURE);
  }

  // Set receive timeout to 3 seconds
  struct timeval tv;
  tv.tv_sec = 3;  // 3 second timeout
  tv.tv_usec = 0;
  if (setsockopt(server_fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
    std::cerr << "Failed to set socket timeout: " << strerror(errno)
              << std::endl;
    std::exit(EXIT_FAILURE);
  }

  // Set member variables
  server_fd_ = server_fd;
  txn_id_t next_txn_id = 0;
  running_.test_and_set(std::memory_order_relaxed);

  // Start main loop
  std::cout << "Server is listening on: " << UNIX_SOCKET_PATH << std::endl;
  while (running_.test_and_set(std::memory_order_relaxed)) {
    // Request buffer from buffer pool
    auto recv_buffer_r = db_.buffers_.request();
    if (!recv_buffer_r) {
      std::cout << "No recv buffer available! Retry in 3 seconds." << std::endl;
      std::this_thread::sleep_for(WAIT_TIMEOUT);
      continue;
    }
    // Prepare buffer and client address
    auto recv_buffer = recv_buffer_r.unwrap();
    auto client_addr = sockaddr_un{};
    auto client_addr_len = socklen_t(sizeof(client_addr));
    // Receive request from client
    ssize_t recv_size =
        recvfrom(server_fd_, recv_buffer.buffer().data(), BUFFER_WIDTH, 0,
                 reinterpret_cast<sockaddr*>(&client_addr), &client_addr_len);
    // Check if fd is closed
    if (0 == recv_size) {
      std::cout << "Socket has been shut down. Stop receiving messages."
                << std::endl;
      break;
    }
    // Check if received error
    if (recv_size < 0) {
      std::cerr << "Failed to receive from client: " << strerror(errno)
                << std::endl;
      continue;
    }

    auto recv_size_u = static_cast<std::size_t>(recv_size);
    auto parser = buf_parser(recv_buffer, recv_size_u);
    auto op_r = parser.get_op();
    auto op_id_r = parser.get_op_id();
    if (!op_r || !op_id_r) {
      std::cerr << "Failed to parse operation" << std::endl;
      reply(OpAck::BAD_OP, parser, client_addr, client_addr_len);
      continue;
    }
    auto op = op_r.unwrap();
    switch (op) {
      case OpType::START_TXN: {
        // clear all dead transactions
        while (!ret_ch_.empty()) {
          auto dead_txn = ret_ch_.get().unwrap();
          workers_.remove(dead_txn);
          ret_ch_.pop();
        }
        // Insert new worker (will start as well)
        auto new_worker_ct =
            WorkerContext{&ret_ch_, &db_, next_txn_id, server_fd_};
        auto insert_r = workers_.insert(new_worker_ct);
        // Check if worker was inserted
        if (!insert_r) {
          reply(OpAck::BUSY, parser, client_addr, client_addr_len);
          continue;
        }
        auto& worker_entry = insert_r.unwrap();
        // Send the first job to the worker
        worker_entry.send(
            {recv_buffer, recv_size_u, client_addr, client_addr_len});
        worker_entry.notify_not_empty();
        next_txn_id++;
        // We don't release the buffer here, the worker will take care of it.
        continue;
      }
      case OpType::COMMIT_TXN:
      case OpType::ABORT_TXN:
      case OpType::ADD_TUPLE:
      case OpType::SHARED_READ:
      case OpType::EXCLUSIVE_READ:
      case OpType::YIELD_READ:
      case OpType::PROMOTE:
      case OpType::UPDATE:
      case OpType::DELETE: {
        // Parser the transaction ID
        auto txn_id_r = parser.get_txn();
        if (!txn_id_r) {
          std::cerr << "Failed to parse transaction ID" << std::endl;
          reply(OpAck::BAD_TXN, parser, client_addr, client_addr_len);
          continue;
        }
        // Search for the txn worker
        auto search_r = workers_.search(txn_id_r.unwrap());
        if (!search_r) {
          std::cerr << "Failed to find worker" << std::endl;
          reply(OpAck::BAD_TXN, parser, client_addr, client_addr_len);
          continue;
        }
        // Send the job to the worker
        auto& worker = search_r.unwrap();
        auto send_r = worker.send(
            {recv_buffer, recv_size_u, client_addr, client_addr_len});
        if (send_r != QueueError::None) {
          std::cerr << "Failed to send job to worker" << std::endl;
          reply(OpAck::BUSY, parser, client_addr, client_addr_len);
          continue;
        }
        // If the operation is abort, kill the worker if the worker is waiting.
        if (op == OpType::ABORT_TXN) {
          worker.stop();
        }
        // Notify the worker that there is a job
        worker.notify_not_empty();
        continue;
      }
      default: {
        continue;
      }
    }
  }
  // Running flag is cleared here, stop all workers and join them
  close(server_fd_);
  unlink(UNIX_SOCKET_PATH);
  workers_.clear();
}

void ThreadManager::trait_stop() noexcept {
  running_.clear(std::memory_order_relaxed);
  shutdown(server_fd_, SHUT_RDWR);
}

bool ThreadManager::trait_is_running() noexcept {
  if (!running_.test_and_set(std::memory_order_relaxed)) {
    running_.clear(std::memory_order_relaxed);
    return false;
  }
  return true;
}

}  // namespace PawnDB
