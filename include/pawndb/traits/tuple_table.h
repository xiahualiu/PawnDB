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
enum class TableError {
  None,    /**< Operation successful */
  Full,    /**< Table at capacity */
  Timeout, /**< Lock wait timeout */
  NotFound /**< Entry not found */
};

/**
 * @brief CRTP interface for tuple table implementations
 * @tparam Derived Class implementing table interface
 * @tparam KeyType Type of keys for indexing
 * @tparam Tuple Type of stored tuples
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
template <typename Derived, typename KeyType, typename Tuple>
class TupleTableTrait {
 public:
  /** @brief Result type for fetch operations */
  using FetchR = Result<std::pair<Tuple*, KeyType>, TableError>;

  /**
   * @brief Insert tuple into table
   * @param tuple Tuple to insert
   * @return FetchR Success: {tuple,key}, Error: code
   */
  FetchR insert(const Tuple& tuple) noexcept {
    return static_cast<Derived*>(this)->trait_insert(tuple);
  }

  /**
   * @brief Remove tuple by key
   * @param key Key of tuple to remove
   */
  void remove(const KeyType& key) noexcept {
    return static_cast<Derived*>(this)->trait_remove(key);
  }

  /**
   * @brief Write tuple at key
   * @param key Target key
   * @param tuple New tuple value
   */
  void write(const KeyType& key, const Tuple& tuple) noexcept {
    return static_cast<Derived*>(this)->trait_write(key, tuple);
  }

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
  TableError promote(const KeyType& key) noexcept {
    return static_cast<Derived*>(this)->trait_promote(key);
  }

  /**
   * @brief Yield a shared lock
   * @param key Key to yield
   */
  void yield(const KeyType& key) noexcept {
    return static_cast<Derived*>(this)->trait_yield(key);
  }

  /**
   * @brief Release any held lock
   * @param key Key to release
   */
  void release(const KeyType& key) noexcept {
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
