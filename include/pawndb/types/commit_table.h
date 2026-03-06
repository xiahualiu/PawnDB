#ifndef PAWNDB_TYPES_COMMIT_TABLE_H
#define PAWNDB_TYPES_COMMIT_TABLE_H

#include <array>
#include <cstddef>

#include "pawndb/params.h"
#include "pawndb/traits/commit.h"
#include "pawndb/traits/container.h"
#include "pawndb/traits/copy.h"
#include "pawndb/traits/parser.h"
#include "pawndb/traits/queue.h"
#include "pawndb/traits/sized.h"
#include "pawndb/types/buffer_table.h"
#include "pawndb/types/table_tuple_key.h"

namespace PawnDB {

class CommitIt;

/**
 * @brief Entry in commit table storing operation details
 *
 * Stores buffer reference, key, operation type and hash table flags.
 * Implements hash and copy operations for table storage.
 */
class CommitEntry : public CommitTrait<CommitEntry>,
                    public CopyTrait<CommitEntry> {
 public:
  /** @brief Key type alias */
  using key_t = TableTupleKey;

  /** @brief Default constructor creates invalid entry */
  constexpr CommitEntry() noexcept
      : buffer_(), key_(), op_(OpType::MAX_OP_VALUE) {}

  /**
   * @brief Construct entry with values
   * @param key Table-tuple key
   * @param op Operation type
   * @param buffer Associated buffer
   */
  CommitEntry(const TableTupleKey& key, OpType op,
              const BufferRef& buffer) noexcept;

  // Copyable
  CommitEntry(const CommitEntry& other) noexcept;
  CommitEntry& operator=(const CommitEntry& other) noexcept;

  // CopyTrait Implementation
  /** @brief Create deep copy */
  CommitEntry trait_copy() const noexcept;

  /** @brief Copy from another entry */
  void trait_copy_from(const CommitEntry& other) noexcept;

  // CommitTrait Implementation
  /** @brief Get buffer reference */
  BufferRef trait_buffer() const noexcept;

  /** @brief Get table-tuple key */
  const TableTupleKey& trait_key() const noexcept;

  /** @brief Get operation type */
  OpType trait_op() const noexcept;

 private:
  BufferRef buffer_;  /**< Associated buffer */
  TableTupleKey key_; /**< Table-tuple key */
  OpType op_;         /**< Operation type */

  friend class CommitIt;
  friend class CommitTable;
};

/**
 * @brief Fixed-size hash table for storing transaction commit entries
 *
 * Features:
 * - O(1) lookup and insertion
 * - Iterator support for traversing entries
 * - Fixed maximum size per transaction
 *
 * Implemented traits:
 * - TableTrait: Hash table operations
 * - IterTrait: Iterator support
 * - Sized: Size tracking
 * - Container: Capacity operations
 */
class CommitTable : public QueueTrait<CommitTable, CommitEntry>,
                    public SizedTrait<CommitTable>,
                    public ContainerTrait<CommitTable> {
  /** @brief Maximum entries per transaction */
  static constexpr std::size_t N = MAX_COMMIT_PER_TRANSACTION;

 public:
  /** @brief Default constructor initializes empty table */
  constexpr CommitTable() noexcept : commits_(), head_(0), tail_(0), size_(0) {}

  // Not copyable
  CommitTable(const CommitTable& other) noexcept = delete;
  CommitTable& operator=(const CommitTable& other) noexcept = delete;

  // QueueTrait Implementation
  /** @brief Get commit entry without blocking */
  queue_r trait_get() noexcept;

  /** @brief Wait for and get commit entry */
  // queue_r trait_recv() noexcept;

  /** @brief Add commit entry to queue */
  QueueError trait_send(const CommitEntry& entry) noexcept;

  /** @brief Remove front entry */
  void trait_pop() noexcept;

  /** @brief Clear all entries */
  void trait_clear() noexcept;

  /** @brief Signal entry available */
  // void trait_notify_not_empty() noexcept;

  // Sized Implementation
  /** @brief Get number of entries */
  std::size_t trait_size() const noexcept;

  // Container Implementation
  /** @brief Check if table is empty */
  bool trait_empty() const noexcept;

  /** @brief Check if table is full */
  bool trait_full() const noexcept;

 private:
  std::array<CommitEntry, N> commits_; /**< Entry storage */
  std::size_t head_;                   /**< Read position */
  std::size_t tail_;                   /**< Write position */
  std::size_t size_;                   /**< Number of entries */
};

}  // namespace PawnDB

#endif  // PAWNDB_TYPES_COMMIT_TABLE_H
