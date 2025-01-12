#ifndef PAWNDB_TYPES_LOCK_RECORDS_H
#define PAWNDB_TYPES_LOCK_RECORDS_H

#include <array>

#include "pawndb/params.h"
#include "pawndb/traits/container.h"
#include "pawndb/traits/copy.h"
#include "pawndb/traits/eq.h"
#include "pawndb/traits/hash_table.h"
#include "pawndb/traits/iterator.h"
#include "pawndb/traits/lock_manager.h"
#include "pawndb/traits/sized.h"
#include "pawndb/types/table_tuple_key.h"

namespace PawnDB {

class LockRecordIterator;

/**
 * @brief Lock entry storing lock information
 */
class LockEntry : public CopyTrait<LockEntry> {
 private:
  TableTupleKey key; /**< Tuple identifier */
  LockType type;     /**< Lock mode */
  bool is_used;      /**< Usage flag */
  bool is_deleted;   /**< Deletion flag */

  friend class LockRecords;
  friend class LockRecordIterator;

 public:
  using key_t = TableTupleKey; /**< Key type alias */

  /** @brief Default constructor creates invalid entry */
  constexpr LockEntry() noexcept
      : key(), type(LockType::EXCLUSIVE), is_used(false), is_deleted(false) {}

  /** @brief Construct lock entry with values
   *  @param _key Tuple key
   *  @param _type Lock type */
  LockEntry(const TableTupleKey& _key, LockType _type) noexcept;

  /** @brief Copy constructor */
  LockEntry(const LockEntry& other) noexcept;

  /** @brief Copy assignment */
  LockEntry& operator=(const LockEntry& other) noexcept;

  // CopyTrait Implementation
  /** @brief Create deep copy */
  LockEntry trait_clone() const noexcept;

  /** @brief Copy from other lock */
  void trait_copy(const LockEntry& other) noexcept;

  // LockTrait Implementation
  /** @brief Get lock type */
  LockType trait_lock_type() const noexcept;

  /** @brief Get lock key */
  const TableTupleKey& trait_key() const noexcept;
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
 * - HashTableTrait: Lock table operations
 * - IterTrait: Lock scanning
 * - SizedTrait: Lock count tracking
 * - ContainerTrait: Capacity management
 */
class LockRecords : public HashTableTrait<LockRecords, LockEntry>,
                    public LockManagerTrait<LockRecords, TableTupleKey>,
                    public IterTrait<LockRecords, LockRecordIterator>,
                    public SizedTrait<LockRecords>,
                    public ContainerTrait<LockRecords> {
 private:
  constexpr static std::size_t N = MAX_LOCK_PER_TRANSACTION;

  std::array<LockEntry, N> locks_;
  std::size_t size_;
  TableTupleKey next_;

  friend class LockRecordIterator;

 public:
  constexpr LockRecords() noexcept : locks_(), size_(0), next_() {}

  // Non-copyable
  LockRecords(const LockRecords& other) noexcept = delete;
  LockRecords& operator=(const LockRecords& other) noexcept = delete;

  // Mon-Movable
  LockRecords(LockRecords&& other) noexcept = delete;
  LockRecords& operator=(LockRecords&& other) noexcept = delete;

  // HashTableTrait Implementation
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

  /** @brief Update existing lock
   *  @param entry Lock with updated values
   *  @return Error status */
  TableError trait_write(const LockEntry& entry) noexcept;

  // LockManagerTrait Implementation
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
  LockR trait_get_lock(const TableTupleKey& key) noexcept;

  /** @brief Get current number of locks
   *  @return Number of active locks */
  std::size_t trait_size() const noexcept;

  /** @brief Check if no locks held
   *  @return true if no active locks */
  bool trait_empty() const noexcept;

  /** @brief Check if at capacity
   *  @return true if no more locks can be acquired */
  bool trait_full() const noexcept;

  /** @brief Get iterator to first lock
   *  @return Iterator positioned at first valid lock */
  LockRecordIterator trait_begin() const noexcept;

  /** @brief Get end iterator
   *  @return Iterator positioned after last lock */
  LockRecordIterator trait_end() const noexcept;
};

/**
 * @brief Iterator for scanning active locks
 *
 * Features:
 * - Forward-only iteration
 * - Skips deleted/unused entries
 * - Const access to lock entries
 *
 * Example:
 * @code
 * LockRecords locks;
 * for (auto it = locks.begin(); it != locks.end(); ++it) {
 *   const LockEntry& lock = *it;
 *   // Process lock entry
 * }
 * @endcode
 */
class LockRecordIterator
    : public IterTypeTrait<LockRecordIterator, const LockEntry>,
      public CopyTrait<LockRecordIterator>,
      public EqTrait<LockRecordIterator> {
 public:
  /**
   * @brief Construct iterator
   * @param table Parent lock table
   * @param idx Starting position
   */
  LockRecordIterator(const LockRecords* table, const std::size_t idx) noexcept;

  /** @brief Copy constructor */
  LockRecordIterator(const LockRecordIterator& other) noexcept;

  /** @brief Copy assignment */
  LockRecordIterator& operator=(const LockRecordIterator& other) noexcept;

  /** @brief Move to next valid lock */
  LockRecordIterator& trait_next() noexcept;

  /**
   * @brief Get current lock entry
   * @return Reference to current lock
   */
  const LockEntry& trait_deref() noexcept;

  /** @brief Copy iterator */
  void trait_copy(const LockRecordIterator& other) noexcept;

  /** @brief Create a copy */
  LockRecordIterator trait_clone() const noexcept;

  /**
   * @brief Compare iterator positions
   * @param other Iterator to compare with
   * @return true if at same position
   */
  bool trait_equals(const LockRecordIterator& other) const noexcept;

 private:
  /** @brief Advance to next valid entry */
  void advance_to_valid() noexcept;

  const LockRecords* table_; /**< Parent table */
  std::size_t idx_;          /**< Current position */

  friend class LockRecords;
};

}  // namespace PawnDB

#endif  // PAWNDB_TYPES_LOCK_RECORDS_H
