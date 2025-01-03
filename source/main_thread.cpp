/**
 * @file main_thread.cpp
 * @brief Implementation of PawnDB's main thread functionality.
 * @version 0.1
 * @date 2025-01-02
 *
 * This file implements the main thread functionality of PawnDB, which:
 * - Manages worker threads
 * - Handles client connections
 * - Coordinates transaction processing
 * - Manages system resources
 *
 * The main thread acts as the coordinator for all database operations,
 * delegating work to worker threads and managing the overall system state.
 *
 * @copyright MIT License
 */

#include "pawndb/main_thread.h"

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

#include <atomic>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <iostream>

#include "pawndb/buffer_table.h"
#include "pawndb/channel.h"
#include "pawndb/params.h"
#include "pawndb/parser.h"
#include "pawndb/worker_thread.h"

namespace PawnDB {

/**
 * @brief Sends an acknowledgment packet to the client
 *
 * @param _ack Acknowledgment type to send
 * @param _parser Parser containing response data
 * @param _server_fd Server socket file descriptor
 * @param _client_addr Client address structure
 * @param _client_addr_len Length of client address structure
 */
static void packet_ack(OpAck _ack, Parser& _parser, int _server_fd,
                       sockaddr& _client_addr,
                       socklen_t _client_addr_len) noexcept {
  _parser.set_ack(_ack);
  sendto(_server_fd, _parser.buffer().data(), _parser.get_buffer_size(), 0,
         &_client_addr, _client_addr_len);
}

/**
 * @brief Cleans up terminated worker threads
 *
 * Handles worker thread cleanup by:
 * - Receiving terminated transaction IDs
 * - Removing from transaction table
 * - Clearing running flags
 * - Draining job channels
 */
void MainThread::clean_worker() noexcept {
  while (true) {
    auto dead_txn_r = main_ch.get();
    if (!dead_txn_r) {
      break;
    }
    auto dead_txn_id = dead_txn_r.unwrap();
    auto index_r = txn_table.remove(dead_txn_id);
    if (!index_r) {
      std::cerr << "Failed to remove worker" << std::endl;
      std::terminate();
    }
    auto index = index_r.unwrap();
    running_flags[index].clear();
    while (true) {
      auto get_r = job_chs[index].get();
      if (!get_r) {
        break;
      }
      auto job = get_r.unwrap();
      auto parser = Parser(db.buffers[job.buffer_index], job.buffer_size);
      parser.set_ack(OpAck::DEAD_TXN);
      sendto(server_fd, parser.buffer().data(), parser.get_buffer_size(), 0,
             &job.client_addr, job.client_addr_len);
    }
    workers[index].thread.join();
    main_ch.pop();
  }
}

/**
 * @brief Starts the main server thread
 *
 * - Creates Unix domain socket
 * - Binds to configured address
 * - Initializes worker threads
 * - Processes client requests
 * - Handles cleanup on shutdown
 *
 * @throws std::runtime_error on socket/bind failure
 */
void MainThread::start() noexcept {
  server_fd = socket(AF_UNIX, SOCK_DGRAM, 0);
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
  unlink(UNIX_SOCKET_PATH);
  if (errno != 0 && errno != ENOENT) {
    std::cerr << "Failed to unlink existing socket: " << strerror(errno)
              << std::endl;
    std::exit(EXIT_FAILURE);
  }
  if (-1 == bind(server_fd, reinterpret_cast<struct sockaddr*>(&server_addr),
                 sizeof(server_addr))) {
    std::cerr << "Failed to bind server socket: " << strerror(errno)
              << std::endl;
    std::exit(EXIT_FAILURE);
  }
  std::cout << "Server is listening on " << UNIX_SOCKET_PATH << std::endl;
  while (true) {
    auto recv_buf_index = db.buffers.request();
    auto& recv_buffer = db.buffers[recv_buf_index];
    auto client_addr = sockaddr();
    auto client_addr_len = socklen_t();
    ssize_t recv_size =
        recvfrom(server_fd, recv_buffer.data(), recv_buffer.size(), 0,
                 &client_addr, &client_addr_len);
    if (-1 == recv_size) {
      std::cerr << "Failed to receive data: " << strerror(errno) << std::endl;
      std::exit(EXIT_FAILURE);
    }
    std::cout << "Received " << recv_size << " bytes from client" << std::endl;
    auto parser = Parser(recv_buffer, static_cast<buf_size_t>(recv_size));
    auto op_r = parser.get_op();
    auto op_id_r = parser.get_op_id();
    if (!op_r || !op_id_r) {
      std::cerr << "Failed to parse operation" << std::endl;
      packet_ack(OpAck::BAD_OP, parser, server_fd, client_addr,
                 client_addr_len);
      db.buffers.release(recv_buf_index);
      continue;
    }
    auto op = op_r.unwrap();
    switch (op) {
      case OpType::START_TXN: {
        clean_worker();
        auto index_r = txn_table.insert(next_txn_id);
        if (!index_r) {
          std::cerr << "Failed to insert worker" << std::endl;
          packet_ack(OpAck::BUSY, parser, server_fd, client_addr,
                     client_addr_len);
          db.buffers.release(recv_buf_index);
          continue;
        }
        auto index = index_r.unwrap();
        running_flags[index].test_and_set(std::memory_order_relaxed);
        workers[index] =
            WorkerThread(WorkerContext(&running_flags[index], &job_chs[index],
                                       &main_ch, &db, next_txn_id, server_fd));
        next_txn_id++;
        job_chs[index].send({client_addr, client_addr_len,
                             static_cast<buf_size_t>(recv_size),
                             recv_buf_index});
        job_chs[index].notify();
        continue;
      }
      case OpType::COMMIT_TXN:
      case OpType::ABORT_TXN:
      case OpType::ADD_TUPLE:
      case OpType::SHARED_READ:
      case OpType::EXCLUSIVE_READ:
      case OpType::PROMOTE:
      case OpType::UPDATE:
      case OpType::DELETE: {
        auto txn_id_r = parser.get_txn();
        if (!txn_id_r) {
          std::cerr << "Failed to parse transaction ID" << std::endl;
          packet_ack(OpAck::BAD_TXN, parser, server_fd, client_addr,
                     client_addr_len);
          db.buffers.release(recv_buf_index);
          continue;
        }
        auto txn_id = txn_id_r.unwrap();
        auto search_r = txn_table.search(txn_id);
        if (!search_r) {
          std::cerr << "Failed to find worker" << std::endl;
          packet_ack(OpAck::BAD_TXN, parser, server_fd, client_addr,
                     client_addr_len);
          db.buffers.release(recv_buf_index);
          continue;
        }
        auto index = search_r.unwrap();
        if (!running_flags[index].test_and_set(std::memory_order_acquire)) {
          running_flags[index].clear(std::memory_order_release);
          packet_ack(OpAck::DEAD_TXN, parser, server_fd, client_addr,
                     client_addr_len);
          db.buffers.release(recv_buf_index);
          continue;
        }
        auto send_r = job_chs[index].send({client_addr, client_addr_len,
                                           static_cast<buf_size_t>(recv_size),
                                           recv_buf_index});
        if (ChannelError::TableFull == send_r) {
          packet_ack(OpAck::BUSY, parser, server_fd, client_addr,
                     client_addr_len);
          db.buffers.release(recv_buf_index);
          continue;
        }
        if (op == OpType::ABORT_TXN) {
          running_flags[index].clear(std::memory_order_release);
        }
        job_chs[index].notify();
        continue;
      }
      default: {
        continue;
      }
    }
  }
}

}  // namespace PawnDB
