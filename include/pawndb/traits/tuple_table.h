/**
 * @file tuple_table.h
 * @brief CRTP interface for tuple storage with locking
 * @version 0.1
 * @date 2025-01-02
 *
 * Features:
 * - Tuple storage and retrieval
 * - Shared/exclusive locking
 * - Lock promotion/yielding
 * - Thread safety
 */
#ifndef PAWNDB_TRAITS_TUPLE_TABLE_H
#define PAWNDB_TRAITS_TUPLE_TABLE_H

#include "pawndb/result.h"

namespace PawnDB {
/**
 * @brief Table operation error codes
 */
enum class TupleTableError {
  None,    /**< Operation successful */
  Full,    /**< Table at capacity */
  Timeout, /**< Lock wait timeout */
  NotFound /**< Entry not found */
};

/**
 * @brief CRTP interface for tuple table implementations
 * @tparam Derived Class implementing table interface
 *
 * Required implementations:
 * - FetchR trait_insert(const Tuple&)
 * - void trait_remove(const KeyType&)
 * - void trait_write(const KeyType&, const Tuple&)
 * - FetchR trait_wait_shared()
 * - FetchR trait_wait_exclusive()
 * - TableError trait_promote(const KeyType&)
 * - void trait_yield(const KeyType&)
 * - void trait_release(const KeyType&)
 */
template <typename Derived, typename EntryType>
class TupleTableTrait {
 public:
  using entry_type = EntryType;
  using key_type = typename EntryType::key_type;

  /** @brief Result type for fetch operations */
  using FetchR = Result<EntryType&, TupleTableError>;

  /**
   * @brief Wait for shared lock
   * @return FetchR Success: {tuple,key}, Error: Timeout
   */
  FetchR wait_shared() noexcept {
    return static_cast<Derived*>(this)->trait_wait_shared();
  }

  /**
   * @brief Wait for exclusive lock
   * @return FetchR Success: {tuple,key}, Error: Timeout
   */
  FetchR wait_exclusive() noexcept {
    return static_cast<Derived*>(this)->trait_wait_exclusive();
  }

  /**
   * @brief Promote shared to exclusive lock
   * @param key Key to promote
   * @return TableError None or error code
   */
  TupleTableError promote(const key_type& key) noexcept {
    return static_cast<Derived*>(this)->trait_promote(key);
  }

  /**
   * @brief Yield a shared lock
   * @param key Key to yield
   */
  void yield(const key_type& key) noexcept {
    return static_cast<Derived*>(this)->trait_yield(key);
  }

  /**
   * @brief Release any held lock
   * @param key Key to release
   */
  void release(const key_type& key) noexcept {
    return static_cast<Derived*>(this)->trait_release(key);
  }

  /**
   * @brief Notify shared lock waiters
   */
  void notify_s() noexcept {
    return static_cast<Derived*>(this)->trait_notify_s();
  }

  /**
   * @brief Notify exclusive lock waiters
   */
  void notify_x() noexcept {
    return static_cast<Derived*>(this)->trait_notify_x();
  }

  /**
   * @brief Notify table not empty waiters
   */
  void notify_not_empty() noexcept {
    return static_cast<Derived*>(this)->trait_notify_not_empty();
  }

  /**
   * @brief Notify table not full waiters
   */
  void notify_not_full() noexcept {
    return static_cast<Derived*>(this)->trait_notify_not_full();
  }

 protected:
  TupleTableTrait() = default;
  ~TupleTableTrait() = default;
};

}  // namespace PawnDB

#endif  // PAWNDB_TRAITS_TUPLE_TABLE_H
