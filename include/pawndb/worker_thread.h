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

#include "pawndb/buffer.h"
#include "pawndb/channel.h"
#include "pawndb/lock.h"
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
  BufferRef buffer;          /**< Reference to the request buffer */
};

/**
 * @brief Structure representing a commit operation.
 *
 * This structure holds the information needed to commit an operation to the
 * database, including the operation type, table ID, tuple key, and buffer
 * index.
 */
struct Commit {
  OpType op;        /**< The type of operation to commit */
  tp_id_t tbl_id;   /**< The ID of the table to operate on */
  tbl_row_t tp_key; /**< The key of the tuple to operate on */
  BufferRef buffer; /**< Reference to the buffer containing data */
};

/**
 * @brief Channel for passing dead transaction IDs between threads.
 */
using MainChannel = ChannelData<txn_id_t>;

/**
 * @brief Channel for passing jobs between threads.
 */
using JobChannel = ChannelData<Job>;

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
};

/**
 * @brief Runtime state for worker thread execution.
 *
 * Contains the mutable state needed during worker thread execution,
 * including transaction status, commit queue, and lock management.
 */
struct WorkerRuntime {
  std::uint8_t timeout_cnt;
  QueueData<Commit, MAX_COMMIT_PER_TRANSACTION> commits;
  TxnStatus status;
  LockRecords lock_table;
};

/**
 * @brief Class representing a worker thread.
 *
 * Manages the lifecycle of a worker thread that processes database operations.
 * Each worker thread operates independently and maintains its own transaction
 * context.
 */
class WorkerThread : ChannelFunc<Job>,
                     ChannelFunc<txn_id_t>,
                     QueueFunc<Commit, MAX_COMMIT_PER_TRANSACTION>,
                     ParserFunc,
                     BufferFunc,
                     LockFunc {
  // Alias for ambiguous base class functions
  using _job_func = ChannelFunc<Job>;
  using _txn_func = ChannelFunc<txn_id_t>;
  using _cmt_func = QueueFunc<Commit, MAX_COMMIT_PER_TRANSACTION>;
  using _lk_func = LockFunc;

 public:
  /**
   * @brief Main execution function for worker threads.
   *
   * Processes database operations in a dedicated thread context.
   * Handles transaction management, locking, and client communication.
   *
   * @param _context Worker context containing thread configuration
   */
  static void worker_main(const WorkerContext&& _context) noexcept;

  static void job_ack(const OpAck _ack, const WorkerContext& _ct,
                      ParserStruct& _parser, Job& _job) noexcept;

  static void process_add(const WorkerContext& _ct, WorkerRuntime& _rt,
                          ParserStruct& _parser, Job& _job) noexcept;

  static void process_rm(const WorkerContext& _ct, WorkerRuntime& _rt,
                         ParserStruct& _parser, Job& _job) noexcept;

  static void process_shared_read(const WorkerContext& _ct, WorkerRuntime& _rt,
                                  ParserStruct& _parser, Job& _job) noexcept;

  static void process_exclusive_read(const WorkerContext& _ct,
                                     WorkerRuntime& _rt, ParserStruct& _parser,
                                     Job& _job) noexcept;

  static void process_yield(const WorkerContext& _ct, WorkerRuntime& _rt,
                            ParserStruct& _parser, Job& _job) noexcept;

  static void process_update(const WorkerContext& _ct, WorkerRuntime& _rt,
                             ParserStruct& _parser, Job& _job) noexcept;

  static void process_promote(const WorkerContext& _ct, WorkerRuntime& _rt,
                              ParserStruct& _parser, Job& _job) noexcept;

  static void process_commit(const WorkerContext& _ct, WorkerRuntime& _rt,
                             ParserStruct& _parser, Job& _job) noexcept;

  static void release_locks(const WorkerContext& _ct,
                            const WorkerRuntime& _rt) noexcept;

  static void worker_quit(const WorkerContext& _ct,
                          const WorkerRuntime& _rt) noexcept;
};

}  // namespace PawnDB

#endif
