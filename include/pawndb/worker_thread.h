/**
 * @file worker_thread.h
 * @author Xiahua Liu @xiahualiu
 * @brief PawnDB worker class, used for processing transactions.
 * @version 0.1
 * @date 2025-01-02
 *
 * @copyright MIT License
 *
 */

#ifndef PAWNDB_THREAD_WORKER_H
#define PAWNDB_THREAD_WORKER_H

#include <sys/socket.h>

#include <atomic>
#include <cstdint>
#include <thread>

#include "pawndb/channel.h"
#include "pawndb/lock_table.h"
#include "pawndb/params.h"
#include "pawndb/parser.h"
#include "pawndb/schema/demo.h"

namespace PawnDB {

/**
 * @brief Enumeration of possible transaction statuses.
 *
 * Represents the different states a transaction can be in during its lifecycle.
 */
enum class TxnStatus : std::uint8_t {
  GROWING,   /**< Transaction is acquiring locks and performing operations */
  SHRINKING, /**< Transaction is releasing locks */
  COMMITTED, /**< Transaction has successfully completed */
  ABORTED    /**< Transaction has been rolled back */
};

/**
 * @brief Structure representing a job for the worker thread.
 *
 * Contains all necessary information for processing a client request.
 */
struct Job {
  sockaddr client_addr;      /**< Network address of the client */
  socklen_t client_addr_len; /**< Length of the client address structure */
  buf_size_t buffer_size;    /**< Size of the request buffer */
  tbl_row_t buffer_index;    /**< Index into the buffer pool */
};

/**
 * @brief Structure representing a commit operation.
 *
 * This structure holds the information needed to commit an operation to the
 * database, including the operation type, table ID, tuple key, and buffer
 * index.
 */
struct Commit {
  OpType op;              /**< The type of operation to commit */
  tp_id_t tbl_id;         /**< The ID of the table to operate on */
  tbl_row_t tp_key;       /**< The key of the tuple to operate on */
  tbl_row_t buffer_index; /**< The index of the buffer containing the operation
                             data */
};

/**
 * @brief Channel for passing dead transaction IDs between threads.
 */
using MainChannel = Channel<txn_id_t>;

/**
 * @brief Channel for passing jobs between threads.
 */
using JobChannel = Channel<Job>;

/**
 * @brief Context structure containing worker thread state.
 *
 * Holds all necessary references and state for a worker thread to process
 * database operations, including communication channels and transaction info.
 */
struct WorkerContext {
  std::atomic_flag* running;
  JobChannel* job_ch;
  MainChannel* main_ch;
  Database* db;
  txn_id_t txn_id;
  int server_fd;

  /**
   * @brief Constructs a new WorkerContext object.
   *
   * @param _running Pointer to running flag
   * @param _job_ch Pointer to job channel
   * @param _main_ch Pointer to main channel
   * @param _db Pointer to database
   * @param _txn_id Transaction ID
   * @param _server_fd Server file descriptor
   */
  WorkerContext(std::atomic_flag* _running, JobChannel* _job_ch,
                MainChannel* _main_ch, Database* _db, txn_id_t _txn_id,
                int _server_fd) noexcept
      : running(_running),
        job_ch(_job_ch),
        main_ch(_main_ch),
        db(_db),
        txn_id(_txn_id),
        server_fd(_server_fd) {}
};

/**
 * @brief Runtime state for worker thread execution.
 *
 * Contains the mutable state needed during worker thread execution,
 * including transaction status, commit queue, and lock management.
 */
struct WorkerRuntime {
  std::uint8_t timeout_cnt;
  Queue<Commit, MAX_COMMIT_PER_TRANSACTION> commits;
  TxnStatus status;
  LockTable lock_table;
};

/**
 * @brief Main execution function for worker threads.
 *
 * Processes database operations in a dedicated thread context.
 * Handles transaction management, locking, and client communication.
 *
 * @param _context Worker context containing thread configuration
 */
void worker_main(const WorkerContext&& _context) noexcept;

/**
 * @brief Class representing a worker thread.
 *
 * Manages the lifecycle of a worker thread that processes database operations.
 * Each worker thread operates independently and maintains its own transaction
 * context.
 */
class WorkerThread {
 public:
  WorkerThread() = default;

  /**
   * @brief Constructs a worker thread with given context, starting the thread.
   *
   * @param _context Configuration and state for the worker thread
   */
  WorkerThread(const WorkerContext& _context) noexcept {
    thread = std::thread(worker_main, std::move(_context));
  }

  std::thread thread;
};

}  // namespace PawnDB

#endif
