#ifndef PAWNDB_TYPES_COMMIT_TABLE_H
#define PAWNDB_TYPES_COMMIT_TABLE_H

#include <array>
#include <cstddef>

#include "pawndb/params.h"
#include "pawndb/traits/commit.h"
#include "pawndb/traits/commit_manager.h"
#include "pawndb/traits/container.h"
#include "pawndb/traits/copy.h"
#include "pawndb/traits/eq.h"
#include "pawndb/traits/hash.h"
#include "pawndb/traits/hash_table.h"
#include "pawndb/traits/iterator.h"
#include "pawndb/traits/parser.h"
#include "pawndb/traits/sized.h"
#include "pawndb/types/buffer_table.h"
#include "pawndb/types/table_tuple_key.h"

namespace PawnDB {

class CommitIt;

/**
 * @brief Entry in commit table storing operation details
 *
 * Stores buffer reference, key, operation type and status flags.
 * Implements hash and copy operations for table storage.
 */
class CommitEntry : public CommitTrait<CommitEntry>,
                    private HashTrait<CommitEntry>,
                    public CopyTrait<CommitEntry> {
 public:
  /** @brief Key type alias */
  using key_t = TableTupleKey;

  /** @brief Default constructor creates invalid entry */
  constexpr CommitEntry() noexcept
      : buffer(),
        key(),
        op(OpType::MAX_OP_VALUE),
        is_used(false),
        is_deleted(false) {}

  /**
   * @brief Construct entry with values
   * @param key Table-tuple key
   * @param op Operation type
   * @param buffer Associated buffer
   */
  CommitEntry(const TableTupleKey& key, OpType op,
              const BufferRef& buffer) noexcept;

  /** @brief Copy constructor */
  CommitEntry(const CommitEntry& other) noexcept;

  /** @brief Copy assignment */
  CommitEntry& operator=(const CommitEntry& other) noexcept;

  // Not movable
  CommitEntry(CommitEntry&& other) noexcept = delete;
  CommitEntry& operator=(CommitEntry&& other) noexcept = delete;

  // HashTrait Implementation
  /** @brief Compute hash based on key */
  std::size_t trait_hash() const noexcept;

  // CopyTrait Implementation
  /** @brief Create deep copy */
  CommitEntry trait_clone() const noexcept;

  /** @brief Copy from another entry */
  void trait_copy(const CommitEntry& other) noexcept;

  // CommitTrait Implementation
  /** @brief Get buffer reference */
  BufferRef& trait_buffer() noexcept;

  /** @brief Get table-tuple key */
  const TableTupleKey& trait_key() const noexcept;

  /** @brief Get operation type */
  OpType trait_op() const noexcept;

 private:
  BufferRef buffer;  /**< Associated buffer */
  TableTupleKey key; /**< Table-tuple key */
  OpType op;         /**< Operation type */
  bool is_used;      /**< Entry in use flag */
  bool is_deleted;   /**< Entry deleted flag */

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
 * - HashTableTrait: Hash table operations
 * - IterTrait: Iterator support
 * - Sized: Size tracking
 * - Container: Capacity operations
 */
class CommitTable : public CommitManagerTrait<CommitTable, CommitEntry>,
                    private HashTableTrait<CommitTable, CommitEntry>,
                    public IterTrait<CommitTable, CommitIt>,
                    public SizedTrait<CommitTable>,
                    public ContainerTrait<CommitTable> {
 private:
  /** @brief Maximum entries per transaction */
  static constexpr std::size_t N = MAX_COMMIT_PER_TRANSACTION;

  std::array<CommitEntry, N> entries_; /**< Entry storage */
  std::size_t size_;                   /**< Current entry count */

  friend class CommitIt; /**< Allow iterator access */

 public:
  /** @brief Default constructor initializes empty table */
  constexpr CommitTable() noexcept : size_(0) {}

  // Not copyable
  CommitTable(const CommitTable& other) noexcept = delete;
  CommitTable& operator=(const CommitTable& other) noexcept = delete;

  // Not movable
  CommitTable(CommitTable&& other) noexcept = delete;
  CommitTable& operator=(CommitTable&& other) noexcept = delete;

  // CommitManagerTrait Implementation
  /** @brief Add new commit entry
   *  @param entry Entry to add
   *  @return Error status */
  CommitError trait_add_commit(const CommitEntry& entry) noexcept;

  // HashTableTrait Implementation
  /** @brief Insert new commit entry
   *  @param entry Entry to insert
   *  @return Result containing inserted entry or error */
  table_r trait_insert(const CommitEntry& entry) noexcept;

  /** @brief Search for entry by key
   *  @param key Key to search for
   *  @return Result containing found entry or error */
  table_r trait_search(const TableTupleKey& key) noexcept;

  /** @brief Remove entry by key
   *  @param key Key of entry to remove
   *  @return Error status */
  TableError trait_remove(const TableTupleKey& key) noexcept;

  /** @brief Update existing entry
   *  @param entry Entry with updated values
   *  @return Error status */
  TableError trait_write(const CommitEntry& entry) noexcept;

  // IterTrait Implementation
  /** @brief Get iterator to first entry */
  CommitIt trait_begin() const noexcept;

  /** @brief Get iterator to end position */
  CommitIt trait_end() const noexcept;

  // Sized Implementation
  /** @brief Get number of entries */
  std::size_t trait_size() const noexcept;

  // Container Implementation
  /** @brief Check if table is empty */
  bool trait_empty() const noexcept;

  /** @brief Check if table is full */
  bool trait_full() const noexcept;
};

/**
 * @brief Iterator for traversing commit table entries
 *
 * Features:
 * - Forward-only iteration
 * - Skip deleted/unused entries
 * - Const access to entries
 * - Copyable
 *
 * Implemented traits:
 * - IterTypeTrait: Iterator operations
 * - CopyTrait: Copy operations
 * - EqTrait: Iterator comparison
 */
class CommitIt : public IterTypeTrait<CommitIt, const CommitEntry>,
                 public CopyTrait<CommitIt>,
                 public EqTrait<CommitIt> {
 public:
  /** @brief Construct iterator
   *  @param table Pointer to commit table
   *  @param index Starting index */
  CommitIt(const CommitTable* table, std::size_t index) noexcept;

  /** @brief Copy iterator
   *  @param other Iterator to copy */
  CommitIt(const CommitIt& other) noexcept;

  /** @brief Copy assignment operator
   *  @param other Iterator to copy
   *  @return Reference to this iterator */
  CommitIt& operator=(const CommitIt& other) noexcept;

  /** @brief Move iterator to next valid entry */
  CommitIt& trait_next() noexcept;

  /** @brief Get current entry
   *  @return Reference to current commit entry */
  const CommitEntry& trait_deref() const noexcept;

  /** @brief Copy iterator
   *  @param other Iterator to copy */
  void trait_copy(const CommitIt& other) noexcept;

  /** @brief Clone iterator
   *  @return Copy of iterator */
  CommitIt trait_clone() const noexcept;

  /** @brief Compare iterator positions
   *  @param other Iterator to compare with
   *  @return true if iterators point to same position */
  bool trait_equals(const CommitIt& other) const noexcept;

 private:
  /** @brief Advance to next valid entry */
  void advance_to_valid() noexcept;

  const CommitTable* table_; /**< Parent table */
  std::size_t index_;        /**< Current position */

  friend class CommitTable;
};

}  // namespace PawnDB

#endif  // PAWNDB_TYPES_COMMIT_TABLE_H
