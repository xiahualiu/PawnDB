#ifndef PAWNDB_WORKER_TABLE_H
#define PAWNDB_WORKER_TABLE_H

#include <sys/types.h>

#include <array>
#include <atomic>
#include <cstddef>
#include <thread>

#include "pawndb/params.h"
#include "pawndb/schema/demo.h"
#include "pawndb/traits/container.h"
#include "pawndb/traits/copy.h"
#include "pawndb/traits/hash.h"
#include "pawndb/traits/parser.h"
#include "pawndb/traits/sized.h"
#include "pawndb/traits/table.h"
#include "pawndb/traits/thread.h"
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
class WorkerContext : public HashTrait<WorkerContext>,
                      public ThreadTrait<WorkerContext>,
                      private CopyTrait<WorkerContext>,
                      public QueueTrait<WorkerContext, Job> {
 public:
  using key_t = txn_id_t;

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

  // HashTrait Implementation
  /** @brief Compute hash from transaction ID */
  std::size_t trait_hash() const noexcept;

  // ThreadTrait Implementation
  /** @brief Start worker thread */
  void trait_start() noexcept;

  /** @brief Stop worker thread */
  void trait_stop() noexcept;

  /** @brief Join worker thread */
  void trait_join() noexcept;

  /** @brief Check if thread is running */
  bool trait_is_running() noexcept;

  // CopyTrait Implementation
  /** @brief Create deep copy */
  WorkerContext trait_clone() const noexcept;

  /** @brief Copy from other entry */
  void trait_copy(const WorkerContext& other) noexcept;

  // QueueTrait Implementation
  /** @brief Get job without blocking */
  queue_r trait_get() noexcept;

  /** @brief Wait for and get job */
  queue_r trait_recv() noexcept;

  /** @brief Add job to queue */
  QueueError trait_send(const Job& job) noexcept;

  /** @brief Remove front job */
  void trait_pop() noexcept;

  /** @brief Clear all jobs */
  void trait_clear() noexcept;

  /** @brief Signal job available */
  void trait_notify_not_empty() noexcept;

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
class WorkerTable : public TableTrait<WorkerTable, WorkerContext>,
                    public SizedTrait<WorkerTable>,
                    public ContainerTrait<WorkerTable> {
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

  // TableTrait Implementation
  /** @brief Insert new worker entry
   *  @param _entry Entry to insert
   *  @return Result containing reference to inserted entry or error */
  table_r trait_insert(const entry_t& _entry) noexcept;

  /** @brief Search for worker by key
   *  @param _key Key to search for
   *  @return Result containing reference to found entry or error */
  table_r trait_search(const key_t& _key) noexcept;

  /** @brief Remove worker entry
   *  @param _key Key of entry to remove
   *  @return Error status */
  TableError trait_remove(const key_t& _key) noexcept;

  /** @brief Update worker entry
   *  @param _entry Entry with updated values
   *  @return Error status */
  // TableError trait_write(const entry_t& _entry) noexcept;

  // SizedTrait Implementation
  /** @brief Get current number of workers */
  std::size_t trait_size() const noexcept {
    return size_;
  }

  // ContainerTrait Implementation
  /** @brief Check if no workers */
  bool trait_empty() const noexcept {
    return size_ == 0;
  }

  /** @brief Check if at capacity */
  bool trait_full() const noexcept {
    return size_ >= MAX_TRANSACTIONS;
  }

  /** @brief Clear all workers */
  void trait_clear() noexcept;

  /** @brief Worker Table Entry */
  struct Entry {
    WorkerContext context_;
    bool is_used_;
    bool is_deleted_;
  };

 private:
  std::array<Entry, MAX_TRANSACTIONS> table_;
  std::size_t size_;
};

}  // namespace PawnDB

#endif
