#ifndef PAWNDB_TYPES_LOCK_RECORDS_H
#define PAWNDB_TYPES_LOCK_RECORDS_H

#include <array>

#include "pawndb/params.h"
#include "pawndb/types/table_tuple_key.h"

namespace PawnDB {

class LockRecordIterator;

/**
 * @brief Lock entry storing lock information
 */
class LockEntry {
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

  /** @brief Get lock type */
  LockType lock_type() const noexcept;

  /** @brief Get lock key */
  const TableTupleKey& key() const noexcept;

  /** @brief Create deep copy */
  LockEntry copy() const noexcept;

  /** @brief Copy from other lock */
  void copy_from(const LockEntry& other) noexcept;

 private:
  TableTupleKey key_; /**< Tuple identifier */
  LockType type_;     /**< Lock mode */
  bool is_used_;      /**< Usage flag */
  bool is_deleted_;   /**< Deletion flag */

  friend class LockRecords;
  friend class LockRecordIterator;
};

/**
 * @brief Fixed-size table storing transaction locks
 *
 * Features:
 * - O(1) lock acquisition and release
 * - Iterator support for lock scanning
 * - Fixed maximum locks per transaction
 * - Thread-safe operations
 */
class LockRecords {
 public:
  /** @brief Result type for table operations */
  using table_r = Result<LockEntry&, TableError>;

  /** @brief Result type for lock queries */
  using LockR = Result<const LockEntry&, LockError>;

 private:
  /** @brief Maximum locks per transaction */
  constexpr static std::size_t N = MAX_LOCK_PER_TRANSACTION;

 public:
  /** @brief Initialize empty lock table */
  constexpr LockRecords() noexcept : locks_(), size_(0), next_() {}

  // Non-copyable
  LockRecords(const LockRecords& other) noexcept = delete;
  LockRecords& operator=(const LockRecords& other) noexcept = delete;

  /** @brief Add new lock
   *  @param key Key to lock
   *  @param type Lock type
   *  @return Result with lock or error */
  LockError add_lock(const TableTupleKey& key, LockType type) noexcept;

  /** @brief Remove existing lock
   *  @param key Key to unlock
   *  @return Error status */
  LockError rm_lock(const TableTupleKey& key) noexcept;

  /** @brief Promote shared lock to exclusive
   *  @param key Key to promote
   *  @return Result with lock or error */
  LockError promote_lock(const TableTupleKey& key) noexcept;

  /** @brief Get lock information
   *  @param key Key to query
   *  @return Result with lock or error */
  LockR get_lock(const TableTupleKey& key) noexcept;

  /** @brief Get iterator to first lock
   *  @return Iterator positioned at first valid lock */
  LockRecordIterator begin() const noexcept;

  /** @brief Get end iterator
   *  @return Iterator positioned after last lock */
  LockRecordIterator end() const noexcept;

  /** @brief Insert new lock entry
   *  @param entry Lock entry to insert
   *  @return Result containing reference to inserted entry or error */
  table_r insert(const LockEntry& entry) noexcept;

  /** @brief Search for lock by key
   *  @param key Key to search for
   *  @return Result containing reference to found entry or error */
  table_r search(const TableTupleKey& key) noexcept;

  /** @brief Remove lock by key
   *  @param key Key of lock to remove
   *  @return Error status */
  TableError remove(const TableTupleKey& key) noexcept;

  /** @brief Clear all locks */
  void clear() noexcept;

  /** @brief Get current number of locks
   *  @return Number of active locks */
  std::size_t size() const noexcept;

  /** @brief Check if no locks held
   *  @return true if no active locks */
  bool empty() const noexcept;

  /** @brief Check if at capacity
   *  @return true if no more locks can be acquired */
  bool full() const noexcept;

 private:
  std::array<LockEntry, N> locks_; /**< Lock storage */
  std::size_t size_;               /**< Current lock count */
  TableTupleKey next_;             /**< Next free key */

  friend class LockRecordIterator;
};

/**
 * @brief Iterator for scanning active locks
 *
 * Features:
 * - Forward-only iteration
 * - Skips deleted/unused entries
 * - Const access to lock entries
 */
class LockRecordIterator {
 public:
  /**
   * @brief Construct iterator
   * @param table Parent lock table
   * @param idx Starting position
   */
  LockRecordIterator(const LockRecords* table, const std::size_t idx) noexcept;

  // Copyable
  LockRecordIterator(const LockRecordIterator& other) noexcept;
  LockRecordIterator& operator=(const LockRecordIterator& other) noexcept;

  /** @brief Move to next valid lock */
  LockRecordIterator& next() noexcept;

  /**
   * @brief Get current lock entry
   * @return Reference to current lock
   */
  const LockEntry& deref() noexcept;

  /** @brief Copy iterator */
  void copy_from(const LockRecordIterator& other) noexcept;

  /** @brief Create a copy */
  LockRecordIterator copy() const noexcept;

  /**
   * @brief Compare iterator positions
   * @param other Iterator to compare with
   * @return true if at same position
   */
  bool equals(const LockRecordIterator& other) const noexcept;

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

  const LockRecords* table_; /**< Parent table */
  std::size_t idx_;          /**< Current position */

  friend class LockRecords;
};

}  // namespace PawnDB

#endif  // PAWNDB_TYPES_LOCK_RECORDS_H