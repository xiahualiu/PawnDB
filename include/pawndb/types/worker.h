#ifndef PAWNDB_WORKER_H
#define PAWNDB_WORKER_H

#include <atomic>

#include "pawndb/schema/demo.h"
#include "pawndb/traits/thread.h"
#include "pawndb/types/job_channel.h"
#include "pawndb/types/lock_table.h"
#include "pawndb/types/parser.h"
#include "pawndb/types/ret_channel.h"
#include "pawndb/types/rollback_deque.h"
#include "pawndb/types/worker_table.h"

namespace PawnDB {

/**
 * @brief Worker thread implementation
 */
class Worker : public ThreadTrait<Worker> {
 public:
  Worker(WorkerContext* _entry) noexcept;

  /**
   * @brief Enumeration of possible transaction statuses.
   *
   * Represents the different states a transaction can be in during its
   * lifecycle.
   */
  enum class TxnStatus : std::uint8_t {
    GROWING,   /**< Transaction is acquiring locks and performing operations */
    SHRINKING, /**< Transaction is releasing locks */
    COMMITTED, /**< Transaction has successfully completed */
    ABORTED    /**< Transaction has been rolled back */
  };

  // ThreadTrait Implementation
  /** @brief Start worker thread */
  void trait_start() noexcept;

  /** @brief Stop worker thread */
  void trait_stop() noexcept;

  /** @brief Join worker thread */
  void trait_join() noexcept;

  /** @brief Check if thread is running */
  bool trait_is_running() noexcept;

 private:
  // Reply functions
  void reply(OpAck _ack, Parser& _parser, std::size_t _size,
             Job& _job) noexcept;

  /** @brief Worker quit function */
  void worker_quit() noexcept;

  /** @brief Release all locks */
  void release_locks() noexcept;

  /** @brief Clear commit table */
  void clear_commit_table() noexcept;

  // Process functions
  void process_commit(Parser& _parser, Job& _job) noexcept;
  void process_add(Parser& _parser, Job& _job) noexcept;
  void process_shared_read(Parser& _parser, Job& _job) noexcept;
  void process_exclusive_read(Parser& _parser, Job& _job) noexcept;
  void process_yield(Parser& _parser, Job& _job) noexcept;
  void process_promote(Parser& _parser, Job& _job) noexcept;
  void process_update(Parser& _parser, Job& _job) noexcept;
  void process_rm(Parser& _parser, Job& _job) noexcept;

  std::atomic_flag& running_;   /**< Running flag */
  JobChannel& job_ch_;          /**< Job channel */
  RetChannel& ret_ch_;          /**< Return channel */
  Database& db_;                /**< Database instance */
  WorkerContext::key_t txn_id_; /**< Transaction ID */
  const int fd_;                /**< Server socket */

  std::uint8_t timeout_cnt_;  /**< Timeout counter */
  RollbackDeque rollback_dq_; /**< Commit buffer table */
  TxnStatus status_;          /**< Current transaction status */
  LockTable lock_table_;    /**< Lock table */
};

}  // namespace PawnDB

#endif  // PAWNDB_WORKER_H
