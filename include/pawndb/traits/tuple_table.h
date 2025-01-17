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
  using key_t = typename EntryType::key_t;

  /** @brief Result type for fetch operations */
  using ttable_r = Result<EntryType&, TupleTableError>;

  /**
   * @brief Wait for shared lock
   * @return FetchR Success: {tuple,key}, Error: Timeout
   */
  ttable_r wait_shared() noexcept {
    return static_cast<Derived*>(this)->trait_wait_shared();
  }

  /**
   * @brief Wait for exclusive lock
   * @return FetchR Success: {tuple,key}, Error: Timeout
   */
  ttable_r wait_exclusive() noexcept {
    return static_cast<Derived*>(this)->trait_wait_exclusive();
  }

  /**
   * @brief Promote shared to exclusive lock
   * @param _key Key to promote
   * @return TableError None or error code
   */
  TupleTableError promote(const key_t& _key) noexcept {
    return static_cast<Derived*>(this)->trait_promote(_key);
  }

  /**
   * @brief Release any held lock
   * @param _key Key to release
   */
  void release(const key_t& _key) noexcept {
    return static_cast<Derived*>(this)->trait_release(_key);
  }

  /**
   * @brief Notify new shared lock can be acquired, usually done after a
   * transaction is finished.
   */
  void notify_shared() noexcept {
    return static_cast<Derived*>(this)->trait_notify_shared();
  }

  /**
   * @brief Notify new shared lock can be acquired, usually done after a
   * transaction is finished.
   */
  void notify_exclusive() noexcept {
    return static_cast<Derived*>(this)->trait_notify_exclusive();
  }

  /**
   * @brief Notify table not empty
   */
  void notify_not_empty() noexcept {
    return static_cast<Derived*>(this)->trait_notify_not_empty();
  }

  /**
   * @brief Notify table not full
   */
  void notify_not_full() noexcept {
    return static_cast<Derived*>(this)->trait_notify_not_full();
  }

 protected:
  // Protected constructor and destructor
  TupleTableTrait() = default;
  ~TupleTableTrait() = default;
};

}  // namespace PawnDB

#endif  // PAWNDB_TRAITS_TUPLE_TABLE_H
