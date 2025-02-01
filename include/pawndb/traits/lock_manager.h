#ifndef PAWNDB_TRAITS_LOCK_MANAGER_H
#define PAWNDB_TRAITS_LOCK_MANAGER_H

#include <cstdint>
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

enum class LockType : std::uint8_t {
  SHARED,   /**< SHARED read lock */
  EXCLUSIVE /**< Exclusive write lock */
};

/**
 * @brief CRTP interface for lock implementations
 * @tparam Derived The derived lock class
 *
 * The derived class must implement the following methods:
 * - `trait_lock_type()`: Returns the type of the lock.
 * - `trait_key()`: Returns the key associated with the lock.
 */
template <typename Derived>
class LockTrait {
 public:
  /** @brief Get lock type */
  LockType lock_type() const noexcept {
    return derived().trait_lock_type();
  }

  /** @brief Get table-tuple key */
  TableTupleKey key() const noexcept {
    return derived().trait_key();
  }

 protected:
  // Protected constructor and destructor
  LockTrait() = default;
  ~LockTrait() = default;

  // CRTP constructor
  Derived& derived() noexcept {
    return static_cast<Derived&>(*this);
  }

  const Derived& derived() const noexcept {
    return static_cast<const Derived&>(*this);
  }
};

/**
 * @brief CRTP interface for lock manager implementations
 * @tparam Derived The derived lock manager class
 * @tparam KeyType The type of the key used for locking
 *
 * The derived class must implement the following methods:
 * - `trait_add_lock(const KeyType&, LockType)`: Adds a new lock.
 * - `trait_rm_lock(const KeyType&)`: Removes an existing lock.
 * - `trait_promote_lock(const KeyType&)`: Promotes a shared lock to exclusive.
 * - `trait_get_lock(const KeyType&)`: Retrieves lock information.
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
    return derived().trait_add_lock(key, type);
  }

  /**
   * @brief Remove existing lock
   * @param key Table-tuple key
   * @return Error status
   */
  LockError rm_lock(const KeyType& key) noexcept {
    return derived().trait_rm_lock(key);
  }

  /**
   * @brief Promote shared lock to exclusive
   * @param key Table-tuple key
   * @return Result with lock or error
   */
  LockError promote_lock(const KeyType& key) noexcept {
    return derived().trait_promote_lock(key);
  }

  /**
   * @brief Get lock information
   * @param key Table-tuple key
   * @return Result with lock or error
   */
  LockR get_lock(const KeyType& key) noexcept {
    return derived().trait_get_lock(key);
  }

 protected:
  // Protected constructor and destructor
  LockManagerTrait() = default;
  ~LockManagerTrait() = default;

  // CRTP constructor
  Derived& derived() noexcept {
    return static_cast<Derived&>(*this);
  }

  const Derived& derived() const noexcept {
    return static_cast<const Derived&>(*this);
  }
};

}  // namespace PawnDB

#endif  // PAWNDB_TRAITS_LOCK_MANAGER_H
