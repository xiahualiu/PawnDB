#ifndef PAWNDB_TYPES_COMMIT_TABLE_H
#define PAWNDB_TYPES_COMMIT_TABLE_H

#include <array>
#include <cstddef>

#include "pawndb/params.h"
#include "pawndb/types/buffer_table.h"
#include "pawndb/types/parser.h"
#include "pawndb/types/unique_key.h"

namespace PawnDB {

/**
 * @brief Entry in commit table storing operation details
 *
 * Stores buffer reference, key, operation type and hash table flags.
 * Implements hash and copy operations for table storage.
 */
class commit_entry {
 public:
  /** @brief Key type alias */
  using key_t = unique_key;

  /** @brief Default constructor creates invalid entry */
  constexpr commit_entry() noexcept
      : buffer_(), key_(), op_(OpType::MAX_OP_VALUE) {}

  /**
   * @brief Construct entry with values
   * @param key Table-tuple key
   * @param op Operation type
   * @param buffer Associated buffer
   */
  commit_entry(const unique_key& key, OpType op,
               const buf_ref& buffer) noexcept;

  // Copyable
  commit_entry(const commit_entry& other) noexcept;
  commit_entry& operator=(const commit_entry& other) noexcept;

  /** @brief Get buffer reference */
  buf_ref buf() const noexcept;

  /** @brief Get table-tuple key */
  const unique_key& key() const noexcept;

  /** @brief Get operation type */
  OpType op() const noexcept;

 private:
  buf_ref buffer_; /**< Associated buffer */
  unique_key key_; /**< Table-tuple key */
  OpType op_;      /**< Operation type */

  friend class commit_table;
};

/**
 * @brief Fixed-size hash table for storing transaction commit entries
 *
 * Features:
 * - O(1) lookup and insertion
 * - Iterator support for traversing entries
 * - Fixed maximum size per transaction
 */
class commit_table {
 private:
  /** @brief Maximum entries per transaction */
  static constexpr std::size_t N = MAX_COMMIT_PER_TRANSACTION;

 public:
  /** @brief Result type for queue operations */
  using queue_r = Result<commit_entry, QueueError>;

  /** @brief Default constructor initializes empty table */
  constexpr commit_table() noexcept
      : commits_(), head_(0), tail_(0), size_(0) {}

  // Not copyable
  commit_table(const commit_table& other) noexcept = delete;
  commit_table& operator=(const commit_table& other) noexcept = delete;

  /** @brief Get commit entry without blocking */
  queue_r get() noexcept;

  /** @brief Add commit entry to queue */
  QueueError send(const commit_entry& entry) noexcept;

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
  std::array<commit_entry, N> commits_; /**< Entry storage */
  std::size_t head_;                    /**< Read position */
  std::size_t tail_;                    /**< Write position */
  std::size_t size_;                    /**< Number of entries */
};

}  // namespace PawnDB

#endif  // PAWNDB_TYPES_COMMIT_TABLE_H
