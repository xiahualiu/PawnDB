#ifndef PAWNDB_WORKER_H
#define PAWNDB_WORKER_H

#include <atomic>

#include "pawndb/schema/demo.h"
#include "pawndb/types/commit_queue.h"
#include "pawndb/types/job_buf.h"
#include "pawndb/types/job_channel.h"
#include "pawndb/types/lock_list.h"
#include "pawndb/types/ret_channel.h"
#include "pawndb/types/worker_table.h"

namespace PawnDB {

/**
 * @brief Worker thread implementation
 */
class Worker {
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

  /** @brief Start worker thread */
  void start_() noexcept;

  /** @brief Stop worker thread */
  void stop_() noexcept;

  /** @brief Join worker thread */
  void join_() noexcept;

  /** @brief Check if thread is running */
  bool is_running_() noexcept;

 private:
  // Reply functions
  void reply(OpAck _ack, job_buf& _parser, std::size_t _size,
             job& _job) noexcept;

  /** @brief Worker quit function */
  void worker_quit() noexcept;

  /** @brief Release all locks */
  void release_locks() noexcept;

  /** @brief Clear commit queue */
  void clear_commit_queue() noexcept;

  // Process functions
  void process_commit(job_buf& _parser, job& _job) noexcept;
  void process_add(job_buf& _parser, job& _job) noexcept;
  void process_shared_read(job_buf& _parser, job& _job) noexcept;
  void process_exclusive_read(job_buf& _parser, job& _job) noexcept;
  void process_yield(job_buf& _parser, job& _job) noexcept;
  void process_promote(job_buf& _parser, job& _job) noexcept;
  void process_update(job_buf& _parser, job& _job) noexcept;
  void process_rm(job_buf& _parser, job& _job) noexcept;

  std::atomic_flag& running_;   /**< Running flag */
  job_channel& job_ch_;         /**< Job channel */
  ret_channel& ret_ch_;         /**< Return channel */
  Database& db_;                /**< Database instance */
  WorkerContext::key_t txn_id_; /**< Transaction ID */
  const int fd_;                /**< Server socket */

  std::uint8_t timeout_cnt_;  /**< Timeout counter */
  commit_queue commit_queue_; /**< Commit buffer queue */
  TxnStatus status_;          /**< Current transaction status */
  lock_list lock_list_;       /**< Lock table */
};

}  // namespace PawnDB

#endif  // PAWNDB_WORKER_H