#ifndef PAWNDB_WORKER_TABLE_H
#define PAWNDB_WORKER_TABLE_H

#include <sys/types.h>

#include <array>
#include <atomic>
#include <cstddef>
#include <thread>

#include "pawndb/params.h"
#include "pawndb/schema/demo.h"
#include "pawndb/types/job_channel.h"
#include "pawndb/types/parser.h"
#include "pawndb/types/ret_channel.h"

namespace PawnDB {

/** @brief Fixed-size table managing worker threads
 *
 * Features:
 * - O(1) hash-based lookup
 * - Thread lifecycle management
 * - Size tracking */
class WorkerContext {
 public:
  using key_t = txn_id_t;

  /** @brief Result type for queue operations */
  using queue_r = Result<Job, QueueError>;

  /** @brief Construct a new Worker Entry object */
  WorkerContext() noexcept;

  /** @brief Destructor */
  ~WorkerContext() noexcept;

  /** @brief Construct a new Worker Entry object */
  WorkerContext(RetChannel* _ret_ch, Database* _db, txn_id_t _txn_id,
                int _fd) noexcept;

  /** @brief Construct a new Worker Entry object */
  WorkerContext(const WorkerContext& _other) noexcept;

  /** @brief Copy from other entry */
  WorkerContext& operator=(const WorkerContext& _other) noexcept;

  /** @brief Compute hash from transaction ID */
  std::size_t hash_() const noexcept;

  /** @brief Start worker thread */
  void start_() noexcept;

  /** @brief Stop worker thread */
  void stop_() noexcept;

  /** @brief Join worker thread */
  void join_() noexcept;

  /** @brief Check if thread is running */
  bool is_running_() noexcept;

  /** @brief Create deep copy */
  WorkerContext copy_() const noexcept;

  /** @brief Copy from other entry */
  void copy_from_(const WorkerContext& other) noexcept;

  /** @brief Get job without blocking */
  queue_r get_() noexcept;

  /** @brief Wait for and get job */
  queue_r recv_() noexcept;

  /** @brief Add job to queue */
  QueueError send_(const Job& job) noexcept;

  /** @brief Remove front job */
  void pop_() noexcept;

  /** @brief Clear all jobs */
  void clear_() noexcept;

  /** @brief Signal job available */
  void notify_not_empty_() noexcept;

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

  RetChannel* ret_ch_;       /**< Return channel */
  Database* db_;             /**< Database instance */
  std::thread thread_;       /**< Worker thread */
  JobChannel job_ch_;        /**< Job channel */
  key_t txn_id_;             /**< Transaction ID */
  int fd_;                   /**< Server socket */
  std::atomic_flag running_; /**< Running flag */

  friend class WorkerTable;
  friend class Worker;
};

/** @brief Fixed-size table storing worker thread entries
 *
 * Features:
 * - O(1) hash-based lookup
 * - Size tracking */
class WorkerTable {
 public:
  using key_t = WorkerContext::key_t;
  using entry_t = WorkerContext;

  // Default constructor
  WorkerTable() noexcept;

  // Destructor need to join all active threads
  ~WorkerTable() noexcept;

  // Non-copyable
  WorkerTable(const WorkerTable& _other) = delete;
  WorkerTable& operator=(const WorkerTable& _other) = delete;

  /** @brief Result type for table operations */
  using table_r = Result<entry_t&, TableError>;

  /** @brief Insert new worker entry
   *  @param _entry Entry to insert
   *  @return Result containing reference to inserted entry or error */
  table_r insert_(const entry_t& _entry) noexcept;

  /** @brief Search for worker by key
   *  @param _key Key to search for
   *  @return Result containing reference to found entry or error */
  table_r search_(const key_t& _key) noexcept;

  /** @brief Remove worker entry
   *  @param _key Key of entry to remove
   *  @return Error status */
  TableError remove_(const key_t& _key) noexcept;

  /** @brief Update worker entry
   *  @param _entry Entry with updated values
   *  @return Error status */
  // TableError write_(const entry_t& _entry) noexcept;

  /** @brief Get current number of workers */
  std::size_t size_() const noexcept {
    return entry_count_;
  }

  /** @brief Check if no workers */
  bool empty_() const noexcept {
    return entry_count_ == 0;
  }

  /** @brief Check if at capacity */
  bool full_() const noexcept {
    return entry_count_ >= MAX_TRANSACTIONS;
  }

  /** @brief Clear all workers */
  void clear_() noexcept;

  /** @brief Worker Table Entry */
  struct Entry {
    WorkerContext context_;
    bool is_used_;
    bool is_deleted_;
  };

  /** @brief Get raw table data */
  std::array<Entry, MAX_TRANSACTIONS>& data_() noexcept {
    return table_;
  }

  /** @brief Set used flag on an entry */
  void set_used_(std::size_t idx, bool val) noexcept {
    table_[idx].is_used_ = val;
  }

  /** @brief Set deleted flag on an entry */
  void set_deleted_(std::size_t idx, bool val) noexcept {
    table_[idx].is_deleted_ = val;
  }

  /** @brief Get entry at index */
  WorkerContext& get_entry_(std::size_t idx) noexcept {
    return table_[idx].context_;
  }

 private:
  std::array<Entry, MAX_TRANSACTIONS> table_;
  std::size_t entry_count_;
};

}  // namespace PawnDB

#endif