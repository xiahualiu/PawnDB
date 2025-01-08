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

#include "pawndb/buffer.h"
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
static void packet_ack(OpAck _ack, ParserStruct& _parser, const int _server_fd,
                       sockaddr& _client_addr,
                       socklen_t _client_addr_len) noexcept {
  ParserFunc::set_ack(_parser, _ack);
  sendto(_server_fd, _parser.data.data(), _parser.size, 0, &_client_addr,
         _client_addr_len);
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
void MainThread::clean_worker(MainContext& _ct) noexcept {
  while (true) {
    auto dead_txn_r = _txn_func::get(_ct.main_ch);
    if (!dead_txn_r) {
      break;
    }
    auto dead_txn_id = dead_txn_r.unwrap();
    auto index_r = remove(_ct.txn_table, dead_txn_id);
    if (!index_r) {
      std::cerr << "Failed to remove worker" << std::endl;
      std::terminate();
    }
    auto index = index_r.unwrap();
    _ct.running_flags[index].clear();
    while (true) {
      auto get_r = _job_func::get(_ct.job_chs[index]);
      if (!get_r) {
        break;
      }
      auto job = get_r.unwrap();
      auto parser = ParserStruct{*job.buffer, job.buffer_size};
      set_ack(parser, OpAck::DEAD_TXN);
      sendto(_ct.server_fd, parser.data.data(), parser.size, 0,
             &job.client_addr, job.client_addr_len);
    }
    _ct.workers[index].join();
    _txn_func::pop(_ct.main_ch);
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
void MainThread::start(MainContext& _ct) noexcept {
  std::cout << "Server is listening on " << UNIX_SOCKET_PATH << std::endl;
  while (true) {
    auto recv_buffer_ref = request(_ct.db->buffers);
    auto recv_buffer = *recv_buffer_ref;
    auto client_addr = sockaddr();
    auto client_addr_len = socklen_t();
    ssize_t recv_size =
        recvfrom(_ct.server_fd, recv_buffer.data(), recv_buffer.size(), 0,
                 &client_addr, &client_addr_len);
    if (0 == recv_size) {
      return;
    }
    if (recv_size < 0) {
      std::cerr << "Failed to receive from client: " << strerror(errno)
                << std::endl;
      continue;
    }
    std::cout << "Received " << recv_size << " bytes from client" << std::endl;
    auto parser = ParserStruct{recv_buffer, static_cast<buf_size_t>(recv_size)};
    auto op_r = get_op(parser);
    auto op_id_r = get_op_id(parser);
    if (!op_r || !op_id_r) {
      std::cerr << "Failed to parse operation" << std::endl;
      packet_ack(OpAck::BAD_OP, parser, _ct.server_fd, client_addr,
                 client_addr_len);
      continue;
    }
    auto op = op_r.unwrap();
    switch (op) {
      case OpType::START_TXN: {
        clean_worker(_ct);
        auto index_r = insert(_ct.txn_table, _ct.next_txn_id);
        if (!index_r) {
          std::cerr << "Failed to insert worker" << std::endl;
          packet_ack(OpAck::BUSY, parser, _ct.server_fd, client_addr,
                     client_addr_len);
          continue;
        }
        auto index = index_r.unwrap();
        _ct.running_flags[index].test_and_set(std::memory_order_relaxed);
        auto worker_context = WorkerContext{&_ct.running_flags[index],
                                            &_ct.job_chs[index],
                                            &_ct.main_ch,
                                            _ct.db,
                                            _ct.next_txn_id,
                                            _ct.server_fd};
        _ct.workers[index] =
            std::thread(WorkerThread::worker_main, std::move(worker_context));
        _ct.next_txn_id++;
        _job_func::send(_ct.job_chs[index],
                        {client_addr, client_addr_len,
                         static_cast<buf_size_t>(recv_size), recv_buffer_ref});
        _ct.job_chs[index].not_empty.notify_one();
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
        auto txn_id_r = get_txn(parser);
        if (!txn_id_r) {
          std::cerr << "Failed to parse transaction ID" << std::endl;
          packet_ack(OpAck::BAD_TXN, parser, _ct.server_fd, client_addr,
                     client_addr_len);
          continue;
        }
        auto txn_id = txn_id_r.unwrap();
        auto search_r = search(_ct.txn_table, txn_id);
        if (!search_r) {
          std::cerr << "Failed to find worker" << std::endl;
          packet_ack(OpAck::BAD_TXN, parser, _ct.server_fd, client_addr,
                     client_addr_len);
          continue;
        }
        auto index = search_r.unwrap();
        if (!_ct.running_flags[index].test_and_set(std::memory_order_acquire)) {
          _ct.running_flags[index].clear(std::memory_order_release);
          packet_ack(OpAck::DEAD_TXN, parser, _ct.server_fd, client_addr,
                     client_addr_len);
          continue;
        }
        auto send_r = _job_func::send(
            _ct.job_chs[index],
            {client_addr, client_addr_len, static_cast<buf_size_t>(recv_size),
             recv_buffer_ref});
        if (ChannelError::TableFull == send_r) {
          packet_ack(OpAck::BUSY, parser, _ct.server_fd, client_addr,
                     client_addr_len);
          continue;
        }
        if (op == OpType::ABORT_TXN) {
          _ct.running_flags[index].clear(std::memory_order_release);
        }
        _ct.job_chs[index].not_empty.notify_one();
        continue;
      }
      default: {
        continue;
      }
    }
  }
}

}  // namespace PawnDB
