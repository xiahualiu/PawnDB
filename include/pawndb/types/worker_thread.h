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

#include "pawndb/params.h"
#include "pawndb/schema/demo.h"
#include "pawndb/traits/thread.h"
#include "pawndb/types/commit_table.h"
#include "pawndb/types/job_channel.h"
#include "pawndb/types/lock_records.h"
#include "pawndb/types/parser.h"
#include "pawndb/types/ret_channel.h"

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

struct WorkerContext {
  std::atomic_flag* running;
  JobChannel* job_ch;
  RetChannel* main_ch;
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
  CommitTable commit_table;
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
class WorkerThread : public ThreadTrait<WorkerThread> {
 public:
  WorkerThread() = default;

  WorkerThread(std::thread* _td, const WorkerContext& _context) noexcept
      : thread_(_td), context_{_context} {}

  void trait_start() noexcept {
    context_.running->test_and_set(std::memory_order_relaxed);
    *thread_ = std::thread(worker_main, context_);
  }

  void setup_context(std::thread* _td, std::atomic_flag* _running,
                     JobChannel* _job_ch, RetChannel* _main_ch, Database* _db,
                     int _server_fd) noexcept {
    thread_ = _td;
    context_.running = _running;
    context_.job_ch = _job_ch;
    context_.main_ch = _main_ch;
    context_.db = _db;
    context_.server_fd = _server_fd;
  }

  bool is_running() const noexcept {
    auto result = context_.running->test_and_set(std::memory_order_acquire);
    if (!result) {
      context_.running->clear(std::memory_order_release);
    }
    return result;
  }

  void trait_stop() noexcept {
    context_.running->clear(std::memory_order_relaxed);
  }

  static void worker_main(const WorkerContext&& _context) noexcept;

  static void job_ack(const OpAck _ack, const WorkerContext& _ct,
                      Parser& _parser, Job& _job) noexcept;

  static void process_add(const WorkerContext& _ct, WorkerRuntime& _rt,
                          Parser& _parser, Job& _job) noexcept;

  static void process_rm(const WorkerContext& _ct, WorkerRuntime& _rt,
                         Parser& _parser, Job& _job) noexcept;

  static void process_shared_read(const WorkerContext& _ct, WorkerRuntime& _rt,
                                  Parser& _parser, Job& _job) noexcept;

  static void process_exclusive_read(const WorkerContext& _ct,
                                     WorkerRuntime& _rt, Parser& _parser,
                                     Job& _job) noexcept;

  static void process_yield(const WorkerContext& _ct, WorkerRuntime& _rt,
                            Parser& _parser, Job& _job) noexcept;

  static void process_update(const WorkerContext& _ct, WorkerRuntime& _rt,
                             Parser& _parser, Job& _job) noexcept;

  static void process_promote(const WorkerContext& _ct, WorkerRuntime& _rt,
                              Parser& _parser, Job& _job) noexcept;

  static void process_commit(const WorkerContext& _ct, WorkerRuntime& _rt,
                             Parser& _parser, Job& _job) noexcept;

  static void release_locks(const WorkerContext& _ct,
                            const WorkerRuntime& _rt) noexcept;

  static void worker_quit(const WorkerContext& _ct,
                          const WorkerRuntime& _rt) noexcept;

  std::thread* thread_;
  WorkerContext context_;
};

}  // namespace PawnDB

#endif
