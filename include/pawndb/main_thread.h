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

#include "pawndb/ds/hash.h"
#include "pawndb/params.h"
#include "pawndb/schema/demo.h"
#include "pawndb/worker_thread.h"

namespace PawnDB {

/**
 * @brief Class representing the main thread.
 */
class MainThread {
 public:
  /**
   * @brief Constructs a new MainThread object.
   *
   * @param _db Reference to the database.
   */
  MainThread(Database& _db) noexcept
      : db(_db),
        workers(),
        job_chs(),
        running_flags(),
        server_fd(),
        main_ch(),
        next_txn_id(0),
        txn_table() {}

  /**
   * @brief Starts the main thread.
   *
   * This function starts the main thread and runs indefinitely.
   */
  [[noreturn]] void start() noexcept;

  /**
   * @brief Cleans up worker threads.
   *
   * This function cleans up the worker threads.
   */
  void clean_worker() noexcept;

 private:
  using TxnTable = Hash<txn_id_t, MAX_TRANSACTIONS>;

  Database& db;
  std::array<WorkerThread, MAX_TRANSACTIONS> workers;
  std::array<JobChannel, MAX_TRANSACTIONS> job_chs;
  std::array<std::atomic_flag, MAX_TRANSACTIONS> running_flags;

  int server_fd;
  MainChannel main_ch;
  txn_id_t next_txn_id;
  TxnTable txn_table;
};

}  // namespace PawnDB

#endif
