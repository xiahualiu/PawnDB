#ifndef PAWNDB_TYPES_LOCK_LIST_H
#define PAWNDB_TYPES_LOCK_LIST_H

#include <array>

#include "pawndb/params.h"
#include "pawndb/result.h"
#include "pawndb/types/unique_key.h"

namespace PawnDB {

/** @brief Lock operation error codes */
enum class LockError {
  None,     /**< Operation successful */
  Full,     /**< Lock table full */
  NotFound, /**< Lock not found */
  Conflict  /**< Lock already held */
};

class LockRecordIterator;

/**
 * @brief Lock entry storing lock information
 */
class lock_entry {
 public:
  using key_t = unique_key; /**< Key type alias */

  /** @brief Default constructor creates invalid entry */
  constexpr lock_entry() noexcept
      : key_(),
        type_(LockType::EXCLUSIVE),
        is_used_(false),
        is_deleted_(false) {}

  /** @brief Construct lock entry with values
   *  @param _key Tuple key
   *  @param _type Lock type */
  lock_entry(const unique_key& _key, LockType _type) noexcept;

  // Copyable
  lock_entry(const lock_entry& other) noexcept;
  lock_entry& operator=(const lock_entry& other) noexcept;

  /** @brief Get lock type */
  LockType lock_type() const noexcept;

  /** @brief Get lock key */
  const unique_key& key() const noexcept;

 private:
  unique_key key_;  /**< Tuple identifier */
  LockType type_;   /**< Lock mode */
  bool is_used_;    /**< Usage flag */
  bool is_deleted_; /**< Deletion flag */

  friend class lock_list;
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
class lock_list {
 public:
  /** @brief Result type for table operations */
  using table_r = Result<lock_entry&, LockError>;

  /** @brief Result type for lock queries */
  using LockR = Result<const lock_entry&, LockError>;

 private:
  /** @brief Maximum locks per transaction */
  constexpr static std::size_t N = MAX_LOCK_PER_TRANSACTION;

 public:
  /** @brief Initialize empty lock table */
  constexpr lock_list() noexcept : locks_(), size_(0), next_() {}

  // Non-copyable
  lock_list(const lock_list& other) noexcept = delete;
  lock_list& operator=(const lock_list& other) noexcept = delete;

  /** @brief Add new lock
   *  @param key Key to lock
   *  @param type Lock type
   *  @return Result with lock or error */
  LockError add_lock(const unique_key& key, LockType type) noexcept;

  /** @brief Remove existing lock
   *  @param key Key to unlock
   *  @return Error status */
  LockError rm_lock(const unique_key& key) noexcept;

  /** @brief Promote shared lock to exclusive
   *  @param key Key to promote
   *  @return Result with lock or error */
  LockError promote_lock(const unique_key& key) noexcept;

  /** @brief Get lock information
   *  @param key Key to query
   *  @return Result with lock or error */
  LockR get_lock(const unique_key& key) noexcept;

  /** @brief Get iterator to first lock
   *  @return Iterator positioned at first valid lock */
  LockRecordIterator begin() const noexcept;

  /** @brief Get end iterator
   *  @return Iterator positioned after last lock */
  LockRecordIterator end() const noexcept;

  /** @brief Insert new lock entry
   *  @param entry Lock entry to insert
   *  @return Result containing reference to inserted entry or error */
  table_r insert(const lock_entry& entry) noexcept;

  /** @brief Search for lock by key
   *  @param key Key to search for
   *  @return Result containing reference to found entry or error */
  table_r search(const unique_key& key) noexcept;

  /** @brief Remove lock by key
   *  @param key Key of lock to remove
   *  @return Error status */
  LockError remove(const unique_key& key) noexcept;

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
  std::array<lock_entry, N> locks_; /**< Lock storage */
  std::size_t size_;                /**< Current lock count */
  unique_key next_;                 /**< Next free key */

  friend class LockRecordIterator;
};

}  // namespace PawnDB

#endif  // PAWNDB_TYPES_LOCK_LIST_H
