/**
 * @file main_thread.cpp
 * @author Xiahua Liu @xiahualiu
 * @brief PawnDB main thread, used for managing worker threads and connections.
 * @version 0.1
 * @date 2025-01-02
 *
 * @copyright MIT License
 *
 */

#ifndef PAWNDB_MAIN_THREAD_H
#define PAWNDB_MAIN_THREAD_H

#include <array>
#include <atomic>
#include <thread>

#include "pawndb/buffer.h"
#include "pawndb/hash.h"
#include "pawndb/params.h"
#include "pawndb/parser.h"
#include "pawndb/schema/demo.h"
#include "pawndb/worker_thread.h"

namespace PawnDB {

using TxnTable = HashTable<txn_id_t, MAX_TRANSACTIONS>;

struct MainContext {
  Database* db;
  std::array<std::thread, MAX_TRANSACTIONS> workers;
  std::array<JobChannel, MAX_TRANSACTIONS> job_chs;
  std::array<std::atomic_flag, MAX_TRANSACTIONS> running_flags;

  int server_fd;
  MainChannel main_ch;
  TxnTable txn_table;
  txn_id_t next_txn_id;
};

/**
 * @brief Class representing the main thread.
 */
class MainThread : public HashFunc<txn_id_t, MAX_TRANSACTIONS>,
                   public ChannelFunc<Job>,
                   public ChannelFunc<txn_id_t>,
                   public ParserFunc,
                   public BufferFunc {
  // Prevent ambiguity with ChannelFunc functions
  using _job_func = ChannelFunc<Job>;
  using _txn_func = ChannelFunc<txn_id_t>;

 public:
  /**
   * @brief Starts the main thread.
   *
   * This function starts the main thread and runs indefinitely.
   */
  static void start(MainContext& _ct) noexcept;

  /**
   * @brief Cleans up worker threads.
   *
   * This function cleans up the worker threads.
   */
  static void clean_worker(MainContext& _ct) noexcept;

 private:
};

}  // namespace PawnDB

#endif
