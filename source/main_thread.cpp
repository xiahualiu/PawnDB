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

#include "pawndb/types/main_thread.h"

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
#include "pawndb/types/buffer_table.h"
#include "pawndb/types/job_channel.h"
#include "pawndb/types/parser.h"
#include "pawndb/types/ret_channel.h"
#include "pawndb/types/worker_table.h"
#include "pawndb/types/worker_thread.h"

namespace PawnDB {

static void packet_ack(OpAck _ack, Parser& _parser, const int _server_fd,
                       sockaddr& _client_addr,
                       socklen_t _client_addr_len) noexcept {
  _parser.set_ack(_ack);
  sendto(_server_fd, _parser.get_buffer(), _parser.get_buffer_size(), 0,
         &_client_addr, _client_addr_len);
}

static void clean_worker(RetChannel& _ret, WorkerTable& _query,
                         MainContext& _ct) noexcept {
  while (true) {
    auto dead_txn_id_r = _ret.get();
    if (!dead_txn_id_r) {
      break;
    }
    auto dead_txn_id = dead_txn_id_r.unwrap();
    auto worker_r = _query.search(dead_txn_id);
    auto worker = worker_r.unwrap();
    worker.worker.context_.running->clear(std::memory_order_relaxed);
    while (true) {
      auto get_r = worker.worker.context_.job_ch->get();
      if (!get_r) {
        break;
      }
      auto job = get_r.unwrap();
      auto parser = Parser(job.buffer.to_array(), job.buffer_size_);
      packet_ack(OpAck::TIMEOUT, parser, _ct.server_fd, job.client_addr_,
                 job.client_addr_len_);
      worker.worker.context_.job_ch->pop();
    }
    worker.worker.thread_->join();
    worker.worker.context_.job_ch->pop();
    _query.remove(dead_txn_id);
  }
}

static void run(MainContext&& _ct) noexcept {
  std::cout << "Server is listening on " << UNIX_SOCKET_PATH << std::endl;

  // Query worker
  WorkerTable worker_query;

  // Pre-allocate worker threads and all associated resources
  RetChannel ret_channel;
  std::array<std::thread, MAX_TRANSACTIONS> workers;
  std::array<std::atomic_flag, MAX_TRANSACTIONS> running_flags;
  std::array<JobChannel, MAX_TRANSACTIONS> job_channels;

  txn_id_t next_txn_id = 0;

  while (true) {
    auto recv_buffer_r = _ct.db->buffers.request();
    if (!recv_buffer_r) {
      std::cout << "No recv buffer available! Retry in 3 seconds."
                << UNIX_SOCKET_PATH << std::endl;
      std::this_thread::sleep_for(WAIT_TIMEOUT);
      continue;
    }
    auto recv_buffer = recv_buffer_r.unwrap();
    auto client_addr = sockaddr();
    auto client_addr_len = socklen_t();
    ssize_t recv_size =
        recvfrom(_ct.server_fd, recv_buffer.to_array().data(), BUFFER_WIDTH, 0,
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
    auto recv_size_u = static_cast<std::size_t>(recv_size);
    auto parser = Parser(recv_buffer.to_array(), recv_size_u);
    auto op_r = parser.get_op();
    auto op_id_r = parser.get_op_id();
    if (!op_r || !op_id_r) {
      std::cerr << "Failed to parse operation" << std::endl;
      packet_ack(OpAck::BAD_OP, parser, _ct.server_fd, client_addr,
                 client_addr_len);
      continue;
    }
    auto op = op_r.unwrap();
    switch (op) {
      case OpType::START_TXN: {
        clean_worker(ret_channel, worker_query, _ct);
        if (worker_query.full()) {
          std::cerr << "Worker table is full" << std::endl;
          packet_ack(OpAck::BUSY, parser, _ct.server_fd, client_addr,
                     client_addr_len);
          continue;
        }
        auto new_worker_entry = WorkerEntry{{}, 0, next_txn_id, false, false};
        auto insert_r = worker_query.insert(new_worker_entry);
        if (!insert_r) {
          std::cerr << "Failed to insert worker" << std::endl;
          packet_ack(OpAck::BUSY, parser, _ct.server_fd, client_addr,
                     client_addr_len);
          continue;
        }
        auto& insert_entry = insert_r.unwrap();
        auto insert_index = insert_entry.index;
        // Populate the worker context and start the worker thread
        insert_entry.worker.setup_context(
            &workers[insert_index], &running_flags[insert_index],
            &job_channels[insert_index], &ret_channel, _ct.db, _ct.server_fd);
        // Start worker thread
        insert_entry.worker.start();
        // Send first job
        insert_entry.worker.context_.job_ch->send(
            {recv_buffer, recv_size_u, client_addr, client_addr_len});
        insert_entry.worker.context_.job_ch->notify_not_empty();
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
          packet_ack(OpAck::BAD_TXN, parser, _ct.server_fd, client_addr,
                     client_addr_len);
          continue;
        }
        auto txn_id = txn_id_r.unwrap();
        auto search_r = worker_query.search(txn_id);
        if (!search_r) {
          std::cerr << "Failed to find worker" << std::endl;
          packet_ack(OpAck::BAD_TXN, parser, _ct.server_fd, client_addr,
                     client_addr_len);
          continue;
        }
        auto& worker = search_r.unwrap();
        if (!worker.worker.is_running()) {
          std::cerr << "Worker is not running" << std::endl;
          packet_ack(OpAck::DEAD_TXN, parser, _ct.server_fd, client_addr,
                     client_addr_len);
          continue;
        }
        auto send_r = worker.worker.context_.job_ch->send(
            {recv_buffer, recv_size_u, client_addr, client_addr_len});
        if (send_r == FIFOError::Full) {
          std::cerr << "Failed to send job to worker" << std::endl;
          packet_ack(OpAck::BUSY, parser, _ct.server_fd, client_addr,
                     client_addr_len);
          continue;
        }
        if (op == OpType::ABORT_TXN) {
          worker.worker.stop();
        }
        worker.worker.context_.job_ch->notify_not_empty();
        continue;
      }
      default: {
        continue;
      }
    }
  }
}

void MainThread::trait_start() noexcept {
  main_thread_ = std::thread(run, std::move(context_));
}

void MainThread::trait_stop() noexcept {
  std::cout << "Shutting down server..." << std::endl;
  shutdown(context_.server_fd, SHUT_RDWR);
  close(context_.server_fd);
  unlink(UNIX_SOCKET_PATH);
  main_thread_.join();
}

bool MainThread::trait_is_running() const noexcept {
  return main_thread_.joinable();
}

}  // namespace PawnDB
