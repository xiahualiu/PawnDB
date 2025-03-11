#ifndef PAWNDB_TYPES_LOCK_RECORDS_H
#define PAWNDB_TYPES_LOCK_RECORDS_H

#include <array>

#include "pawndb/params.h"
#include "pawndb/traits/container.h"
#include "pawndb/traits/copy.h"
#include "pawndb/traits/iterator.h"
#include "pawndb/traits/lock_table.h"
#include "pawndb/traits/sized.h"
#include "pawndb/traits/table.h"
#include "pawndb/types/table_tuple_key.h"

namespace PawnDB {

class LockTableIterator;

/** @brief Lock entry storing lock information */
class LockEntry : public LockTrait<LockEntry>, private CopyTrait<LockEntry> {
 public:
  using key_t = TableTupleKey; /**< Key type alias */

  /** @brief Default constructor creates invalid entry */
  constexpr LockEntry() noexcept
      : key_(),
        type_(LockType::EXCLUSIVE),
        is_used_(false),
        is_deleted_(false) {}

  /** @brief Construct lock entry with values
   *  @param _key Tuple key
   *  @param _type Lock type */
  LockEntry(const TableTupleKey& _key, LockType _type) noexcept;

  // Copyable
  LockEntry(const LockEntry& other) noexcept;
  LockEntry& operator=(const LockEntry& other) noexcept;

  // LockTrait Implementation
  /** @brief Get lock type */
  LockType trait_lock_type() const noexcept;

  /** @brief Get lock key */
  TableTupleKey trait_key() const noexcept;

 private:
  TableTupleKey key_; /**< Tuple identifier */
  LockType type_;     /**< Lock mode */
  bool is_used_;      /**< Usage flag */
  bool is_deleted_;   /**< Deletion flag */

  friend class LockTable;
  friend class LockTableIterator;
};

/**
 * @brief Fixed-size table storing transaction locks
 *
 * Features:
 * - O(1) lock acquisition and release
 * - Iterator support for lock scanning
 * - Fixed maximum locks per transaction
 * - Thread-safe operations
 *
 * Implemented traits:
 * - TableTrait: Lock table operations
 * - IterTrait: Lock scanning
 * - SizedTrait: Lock count tracking
 * - ContainerTrait: Capacity management
 */
class LockTable : public TableTrait<LockTable, LockEntry>,
                    public LockTableTrait<LockTable, TableTupleKey>,
                    public IterTrait<LockTable, LockTableIterator>,
                    public SizedTrait<LockTable>,
                    public ContainerTrait<LockTable> {
  /** @brief Maximum locks per transaction */
  constexpr static std::size_t N = MAX_LOCK_PER_TRANSACTION;

 public:
  /** @brief Initialize empty lock table */
  constexpr LockTable() noexcept : locks_(), size_(0), next_() {}

  // Non-copyable
  LockTable(const LockTable& other) noexcept = delete;
  LockTable& operator=(const LockTable& other) noexcept = delete;

  // LockTableTrait Implementation
  /** @brief Add new lock
   *  @param key Key to lock
   *  @param type Lock type
   *  @return Result with lock or error */
  LockError trait_add_lock(const TableTupleKey& key, LockType type) noexcept;

  /** @brief Remove existing lock
   *  @param key Key to unlock
   *  @return Error status */
  LockError trait_rm_lock(const TableTupleKey& key) noexcept;

  /** @brief Promote shared lock to exclusive
   *  @param key Key to promote
   *  @return Result with lock or error */
  LockError trait_promote_lock(const TableTupleKey& key) noexcept;

  /** @brief Get lock information
   *  @param key Key to query
   *  @return Result with lock or error */
  lock_r trait_get_lock(const TableTupleKey& key) noexcept;

  /** @brief Get iterator to first lock
   *  @return Iterator positioned at first valid lock */
  LockTableIterator trait_begin() const noexcept;

  /** @brief Get end iterator
   *  @return Iterator positioned after last lock */
  LockTableIterator trait_end() const noexcept;

  // TableTrait Implementation
  /** @brief Insert new lock entry
   *  @param entry Lock entry to insert
   *  @return Result containing reference to inserted entry or error */
  table_r trait_insert(const LockEntry& entry) noexcept;

  /** @brief Search for lock by key
   *  @param key Key to search for
   *  @return Result containing reference to found entry or error */
  table_r trait_search(const TableTupleKey& key) noexcept;

  /** @brief Remove lock by key
   *  @param key Key of lock to remove
   *  @return Error status */
  TableError trait_remove(const TableTupleKey& key) noexcept;

  /** @brief Get current number of locks
   *  @return Number of active locks */
  std::size_t trait_size() const noexcept;

  /** @brief Check if no locks held
   *  @return true if no active locks */
  bool trait_empty() const noexcept;

  /** @brief Check if at capacity
   *  @return true if no more locks can be acquired */
  bool trait_full() const noexcept;

 private:
  std::array<LockEntry, N> locks_; /**< Lock storage */
  std::size_t size_;               /**< Current lock count */
  TableTupleKey next_;             /**< Next free key */

  friend class LockTableIterator;
};

/**
 * @brief Iterator for scanning active locks
 *
 * Features:
 * - Forward-only iteration
 * - Skips deleted/unused entries
 * - Const access to lock entries
 */
class LockTableIterator
    : public IterTypeTrait<LockTableIterator, const LockEntry>,
      private CopyTrait<LockTableIterator>,
      public EqTrait<LockTableIterator, LockTableIterator> {
 public:
  /**
   * @brief Construct iterator
   * @param table Parent lock table
   * @param idx Starting position
   */
  LockTableIterator(const LockTable* table, const std::size_t idx) noexcept;

  // Copyable
  LockTableIterator(const LockTableIterator& other) noexcept;
  LockTableIterator& operator=(const LockTableIterator& other) noexcept;

  /** @brief Move to next valid lock */
  LockTableIterator& trait_next() noexcept;

  /**
   * @brief Get current lock entry
   * @return Reference to current lock
   */
  const LockEntry& trait_deref() noexcept;

  /**
   * @brief Compare iterator positions
   * @param other Iterator to compare with
   * @return true if at same position
   */
  bool trait_equals(const LockTableIterator& other) const noexcept;

  /**
   * @brief Get current storage index
   * @return Current index
   */
  std::size_t _test_index() const noexcept {
    return idx_;
  }

 private:
  /** @brief Advance to next valid entry */
  void advance_to_valid() noexcept;

  const LockTable* table_; /**< Parent table */
  std::size_t idx_;          /**< Current position */

  friend class LockTable;
};

}  // namespace PawnDB

#endif  // PAWNDB_TYPES_LOCK_RECORDS_H
