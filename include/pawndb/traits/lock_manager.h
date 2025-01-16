#ifndef PAWNDB_TRAITS_LOCK_MANAGER_H
#define PAWNDB_TRAITS_LOCK_MANAGER_H

#include "pawndb/result.h"
#include "pawndb/types/table_tuple_key.h"

namespace PawnDB {

/**
 * @brief Lock operation error codes
 */
enum class LockError {
  None,        /**< Operation successful */
  NotFound,    /**< Lock not found */
  Conflict,    /**< Lock conflicts */
  InvalidType, /**< Invalid lock type */
  Full         /**< No more locks available */
};

enum class LockType {
  SHARED,   /**< SHARED read lock */
  EXCLUSIVE /**< Exclusive write lock */
};

/**
 * @brief CRTP interface for lock implementations
 * @tparam Derived The derived lock class
 */
template <typename Derived>
class LockTrait {
 public:
  /** @brief Get lock type */
  LockType lock_type() const noexcept {
    return static_cast<const Derived*>(this)->trait_lock_type();
  }

  /** @brief Get table-tuple key */
  const TableTupleKey& key() const noexcept {
    return static_cast<const Derived*>(this)->trait_key();
  }

 protected:
  LockTrait() = default;
  ~LockTrait() = default;
};

/**
 * @brief CRTP interface for lock manager implementations
 */
template <typename Derived, typename KeyType>
class LockManagerTrait {
 public:
  /** @brief Result type for lock operations */
  using LockR = Result<LockType, LockError>;

  /**
   * @brief Add new lock
   * @param key Table-tuple key
   * @param type Lock type
   * @return Result with lock or error
   */
  LockError add_lock(const KeyType& key, LockType type) noexcept {
    return static_cast<Derived*>(this)->trait_add_lock(key, type);
  }

  /**
   * @brief Remove existing lock
   * @param key Table-tuple key
   * @return Error status
   */
  LockError rm_lock(const KeyType& key) noexcept {
    return static_cast<Derived*>(this)->trait_rm_lock(key);
  }

  /**
   * @brief Promote shared lock to exclusive
   * @param key Table-tuple key
   * @return Result with lock or error
   */
  LockError promote_lock(const KeyType& key) noexcept {
    return static_cast<Derived*>(this)->trait_promote_lock(key);
  }

  /**
   * @brief Get lock information
   * @param key Table-tuple key
   * @return Result with lock or error
   */
  LockR get_lock(const KeyType& key) noexcept {
    return static_cast<Derived*>(this)->trait_get_lock(key);
  }

 protected:
  LockManagerTrait() = default;
  ~LockManagerTrait() = default;
};

}  // namespace PawnDB

#endif  // PAWNDB_TRAITS_LOCK_MANAGER_H
