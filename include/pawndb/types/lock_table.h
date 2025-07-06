#ifndef PAWNDB_TYPES_LOCK_RECORDS_H
#define PAWNDB_TYPES_LOCK_RECORDS_H

#include <array>

#include "pawndb/params.h"
#include "pawndb/result.h"
#include "pawndb/types/tuple_uid.h"

namespace PawnDB {

/** @brief Lock type enumeration */
enum class LockType {
  EXCLUSIVE, /**< Exclusive lock */
  SHARED,    /**< Shared lock */
};

/** @brief Lock entry storing lock information */
class LockEntry {
 public:
  using key_t = TupleUID; /**< Key type alias */

  /** @brief Default constructor creates invalid entry */
  constexpr LockEntry() noexcept
      : key_(),
        type_(LockType::EXCLUSIVE),
        is_used_(false),
        is_deleted_(false) {}

  /** @brief Construct lock entry with values
   *  @param _key Tuple UID key
   *  @param _type Lock type */
  LockEntry(const TupleUID& _key, LockType _type) noexcept;

  // Copyable
  LockEntry(const LockEntry& other) noexcept;
  LockEntry& operator=(const LockEntry& other) noexcept;

  /** @brief Get lock type */
  LockType lock_type() const noexcept;

  /** @brief Get lock key */
  TupleUID key() const noexcept;

  /** @brief Calculate hash */
  size_t hash() const noexcept;

 private:
  TupleUID key_;    /**< Tuple identifier */
  LockType type_;   /**< Lock mode */
  bool is_used_;    /**< Usage flag */
  bool is_deleted_; /**< Deletion flag */
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
class LockTable {
  /** @brief Maximum locks per transaction */
  constexpr static std::size_t N = MAX_LOCK_PER_TRANSACTION;

 public:
  enum class LockError {
    OK,               /**< No error */
    TABLE_FULL,       /**< Lock table is full */
    LOCK_NOT_FOUND,   /**< Lock not found */
    LOCK_EXISTS,      /**< Lock already exists */
    INVALID_LOCK_TYPE /**< Invalid lock type */
  };

  using lock_r =
      Result<LockEntry&, LockError>; /**< Result type for lock operations */

  /** @brief Initialize empty lock table */
  constexpr LockTable() noexcept : locks_(), size_(0) {}

  // Non-copyable
  LockTable(const LockTable& other) noexcept = delete;
  LockTable& operator=(const LockTable& other) noexcept = delete;

  /** @brief Add new lock
   *  @param key Key to lock
   *  @param type Lock type
   *  @return Result with lock or error */
  LockError add(const TupleUID& key, LockType type) noexcept;

  /** @brief Remove existing lock
   *  @param key Key to unlock
   *  @return Error status */
  LockError remove(const TupleUID& key) noexcept;

  /** @brief Promote shared lock to exclusive
   *  @param key Key to promote
   *  @return Result with lock or error */
  LockError promote(const TupleUID& key) noexcept;

  /** @brief Get lock information
   *  @param key Key to query
   *  @return Result with lock or error */
  lock_r get_lock(const TupleUID& key) noexcept;

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
};

}

#endif  // PAWNDB_TYPES_LOCK_RECORDS_H
