#ifndef PAWNDB_TYPES_COMMIT_QUEUE_H
#define PAWNDB_TYPES_COMMIT_QUEUE_H

#include <array>
#include <cstddef>

#include "pawndb/params.h"
#include "pawndb/result.h"
#include "pawndb/types/commit.h"

namespace PawnDB {

/**
 * @brief Fixed-size FIFO queue for storing transaction commit entries
 *
 * Features:
 * - O(1) enqueue and dequeue
 * - Iterator support for traversing entries
 * - Fixed maximum size per transaction
 */
class commit_queue {
 private:
  /** @brief Maximum entries per transaction */
  static constexpr std::size_t N = MAX_COMMIT_PER_TRANSACTION;

 public:
  /** @brief Result type for queue operations */
  using queue_r = Result<commit, QueueError>;

  /** @brief Default constructor initializes empty table */
  constexpr commit_queue() noexcept
      : commits_(), head_(0), tail_(0), size_(0) {}

  // Not copyable
  commit_queue(const commit_queue& other) noexcept = delete;
  commit_queue& operator=(const commit_queue& other) noexcept = delete;

  /** @brief Get commit entry without blocking */
  queue_r get() noexcept;

  /** @brief Add commit entry to queue */
  QueueError send(const commit& entry) noexcept;

  /** @brief Remove front entry */
  void pop() noexcept;

  /** @brief Clear all entries */
  void clear() noexcept;

  /** @brief Get number of entries */
  std::size_t size() const noexcept;

  /** @brief Check if table is empty */
  bool empty() const noexcept;

  /** @brief Check if table is full */
  bool full() const noexcept;

 private:
  std::array<commit, N> commits_; /**< Entry storage */
  std::size_t head_;              /**< Read position */
  std::size_t tail_;              /**< Write position */
  std::size_t size_;              /**< Number of entries */
};

}  // namespace PawnDB

#endif  // PAWNDB_TYPES_COMMIT_QUEUE_H
