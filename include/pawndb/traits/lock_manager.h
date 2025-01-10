/**
 * @file lock_table.h
 * @brief CRTP interface for lock management
 * @version 0.1
 * @date 2025-01-02
 *
 * Features:
 * - Shared/exclusive locking
 * - Lock promotion
 * - Key-based locking
 * - Error handling
 */
#ifndef PAWNDB_TRAITS_LOCK_TABLE_H
#define PAWNDB_TRAITS_LOCK_TABLE_H

#include <cstdint>

#include "pawndb/result.h"

namespace PawnDB {

/**
 * @brief Lock types supported by table
 */
enum class LockType : std::uint8_t {
  Shared,   /**< Multiple readers allowed */
  Exclusive /**< Single writer access */
};
/**
 * @brief Lock operation error codes
 */
enum class LockError {
  None,        /**< Operation successful */
  NotFound,    /**< Lock not found */
  Full,        /**< Table at capacity */
  LockConflict /**< Lock request conflicts */
};

/**
 * @brief CRTP interface for lock table implementations
 * @tparam Derived Class implementing lock interface
 * @tparam KeyType Type of keys used for locking
 *
 * Required implementations:
 * - LockError trait_lock(const KeyType&, LockType)
 * - LockError trait_unlock(const KeyType&)
 * - LockError trait_promote(const KeyType&)
 * - LockR trait_get_lock(const KeyType&) const
 */
template <typename Derived, typename KeyType, typename LockEntryType>
class LockManagerTrait {
 public:
  using LockR = Result<const LockEntryType&, LockError>;

  /**
   * @brief Acquire lock on key
   * @param key Key to lock
   * @param type Lock type (shared/exclusive)
   * @return LockError None or error code
   */
  LockError lock(const KeyType& key, LockType type) noexcept {
    return static_cast<Derived*>(this)->trait_lock(key, type);
  }

  /**
   * @brief Release lock on key
   * @param key Key to unlock
   * @return LockError None or NotFound
   */
  LockError unlock(const KeyType& key) noexcept {
    return static_cast<Derived*>(this)->trait_unlock(key);
  }

  /**
   * @brief Promote shared lock to exclusive
   * @param key Key to promote
   * @return LockError None or error code
   */
  LockError promote(const KeyType& key) noexcept {
    return static_cast<Derived*>(this)->trait_promote(key);
  }

  /**
   * @brief Get current lock type for key
   * @param key Key to check
   * @return LockR Success: lock type, Error: NotFound
   */
  LockR get_lock(const KeyType& key) const noexcept {
    return static_cast<const Derived*>(this)->trait_get_lock(key);
  }

 protected:
  LockManagerTrait() = default;
  ~LockManagerTrait() = default;
};

}  // namespace PawnDB

#endif  // PAWNDB_TRAITS_LOCK_TABLE_H
