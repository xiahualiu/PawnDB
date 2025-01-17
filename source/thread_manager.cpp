#include "pawndb/types/thread_manager.h"

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

#include <atomic>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <iostream>

#include "pawndb/params.h"
#include "pawndb/traits/queue.h"
#include "pawndb/types/buffer_table.h"
#include "pawndb/types/parser.h"
#include "pawndb/types/ret_channel.h"
#include "pawndb/types/worker_table.h"

namespace PawnDB {

ThreadManager::ThreadManager(Database* _db, int _server_fd) noexcept
    : db_(*_db), ret_ch_(), workers_(), server_fd_(_server_fd), running_() {}

void ThreadManager::reply(OpAck _ack, Parser& _parser,
                          const sockaddr_un& _client_addr,
                          const socklen_t _client_addr_len) noexcept {
  _parser.set_ack(_ack);
  sendto(server_fd_, _parser.get_buffer().data(), _parser.get_buffer_size(), 0,
         reinterpret_cast<const sockaddr*>(&_client_addr), _client_addr_len);
}

void ThreadManager::trait_start() noexcept {
  std::cout << "Server is listening on " << UNIX_SOCKET_PATH << std::endl;
  txn_id_t next_txn_id = 0;
  running_.test_and_set(std::memory_order_relaxed);
  while (running_.test_and_set(std::memory_order_relaxed)) {
    auto recv_buffer_r = db_.buffers_.request();
    if (!recv_buffer_r) {
      std::cout << "No recv buffer available! Retry in 3 seconds."
                << UNIX_SOCKET_PATH << std::endl;
      std::this_thread::sleep_for(WAIT_TIMEOUT);
      continue;
    }
    auto recv_buffer = recv_buffer_r.unwrap();
    auto client_addr = sockaddr_un{};
    auto client_addr_len = socklen_t(sizeof(client_addr));
    ssize_t recv_size =
        recvfrom(server_fd_, recv_buffer.buffer().data(), BUFFER_WIDTH, 0,
                 reinterpret_cast<sockaddr*>(&client_addr), &client_addr_len);
    std::cout << "Received " << recv_size << " bytes from client" << std::endl;
    std::cout << "Client address: " << client_addr.sun_path << std::endl;
    std::cout << "Client address length: " << client_addr_len << std::endl;
    // Check if fd is closed
    if (0 == recv_size) {
      return;
    }
    if (recv_size < 0) {
      std::cerr << "Failed to receive from client: " << strerror(errno)
                << std::endl;
      recv_buffer.release();
      continue;
    }
    std::cout << "Received " << recv_size << " bytes from client" << std::endl;
    auto recv_size_u = static_cast<std::size_t>(recv_size);
    auto parser = Parser(recv_buffer, recv_size_u);
    auto op_r = parser.get_op();
    auto op_id_r = parser.get_op_id();
    if (!op_r || !op_id_r) {
      std::cerr << "Failed to parse operation" << std::endl;
      reply(OpAck::BAD_OP, parser, client_addr, client_addr_len);
      recv_buffer.release();
      continue;
    }
    auto op = op_r.unwrap();
    switch (op) {
      case OpType::START_TXN: {
        // clear all dead transactions
        while (!ret_ch_.empty()) {
          auto dead_txn_r = ret_ch_.get();
          auto dead_txn = dead_txn_r.unwrap();
          workers_.remove(dead_txn);
        }
        // Insert new worker (will start it as well)
        auto new_worker_entry =
            WorkerContext{&ret_ch_, &db_, next_txn_id, server_fd_};
        auto insert_r = workers_.insert(new_worker_entry);
        // Check if worker was inserted
        if (!insert_r) {
          std::cerr << "Failed to insert worker" << std::endl;
          reply(OpAck::BUSY, parser, client_addr, client_addr_len);
          recv_buffer.release();
          continue;
        }
        auto& worker_entry = insert_r.unwrap();
        // Send the first job to the worker
        worker_entry.send(
            {recv_buffer, recv_size_u, client_addr, client_addr_len});
        worker_entry.notify_not_empty();
        next_txn_id++;
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
        auto txn_id_r = parser.get_txn();
        if (!txn_id_r) {
          std::cerr << "Failed to parse transaction ID" << std::endl;
          reply(OpAck::BAD_TXN, parser, client_addr, client_addr_len);
          recv_buffer.release();
          continue;
        }
        auto txn_id = txn_id_r.unwrap();
        auto search_r = workers_.search(txn_id);
        if (!search_r) {
          std::cerr << "Failed to find worker" << std::endl;
          reply(OpAck::BAD_TXN, parser, client_addr, client_addr_len);
          recv_buffer.release();
          continue;
        }
        auto& worker = search_r.unwrap();
        if (!worker.is_running()) {
          std::cerr << "Worker is not running" << std::endl;
          reply(OpAck::DEAD_TXN, parser, client_addr, client_addr_len);
          recv_buffer.release();
          continue;
        }
        auto send_r = worker.send(
            {recv_buffer, recv_size_u, client_addr, client_addr_len});
        if (send_r != QueueError::None) {
          std::cerr << "Failed to send job to worker" << std::endl;
          reply(OpAck::BUSY, parser, client_addr, client_addr_len);
          recv_buffer.release();
          continue;
        }
        if (op == OpType::ABORT_TXN) {
          worker.stop();
        }
        worker.notify_not_empty();
        continue;
      }
      default: {
        continue;
      }
    }
  }
}

void ThreadManager::trait_stop() noexcept {
  running_.clear(std::memory_order_relaxed);
  // Clear all unfinished workers
  workers_.clear();
}

void ThreadManager::trait_join() noexcept {
  running_.clear(std::memory_order_relaxed);
  // Clear all unfinished workers
  workers_.clear();
}

bool ThreadManager::trait_is_running() noexcept {
  if (!running_.test_and_set(std::memory_order_relaxed)) {
    running_.clear(std::memory_order_relaxed);
    return false;
  }
  return true;
}

}  // namespace PawnDB
