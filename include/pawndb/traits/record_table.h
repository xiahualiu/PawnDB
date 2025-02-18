#ifndef PAWNDB_TRAITS_RECORD_TABLE_H
#define PAWNDB_TRAITS_RECORD_TABLE_H

#include "pawndb/result.h"

namespace PawnDB {
/**
 * @brief Table operation error codes
 */
enum class RecordTableError {
  None,    /**< Operation successful */
  Full,    /**< Table at capacity */
  Timeout, /**< Lock wait timeout */
  NotFound /**< Entry not found */
};

/**
 * @brief CRTP interface for tuple table implementations
 * @tparam Derived Class implementing tuple table interface
 * @tparam EntryType Type of entry in the table
 */
template <typename Derived, typename EntryType>
class RecordTableTrait {
 public:
  using key_t = typename EntryType::key_t;

  /** @brief Result type for fetch operations */
  using rtable_r = Result<EntryType&, RecordTableError>;

  // Read Operations
  rtable_r wait_any_s() noexcept {
    return derived().trait_wait_any_s();
  }

  rtable_r wait_any_x() noexcept {
    return derived().trait_wait_any_x();
  }

  rtable_r wait_s(const key_t& _key) noexcept {
    return derived().trait_wait_any_s(_key);
  }

  rtable_r wait_x(const key_t& _key) noexcept {
    return derived().trait_wait_any_x(_key);
  }

  // Write Operations

  void notify_shared() noexcept {
    return derived().trait_notify_shared();
  }

  void notify_exclusive() noexcept {
    return derived().trait_notify_exclusive();
  }

  void notify_not_empty() noexcept {
    return derived().trait_notify_not_empty();
  }

  void notify_not_full() noexcept {
    return derived().trait_notify_not_full();
  }

  void rollback() noexcept {
    return derived().trait_rollback();
  }

 protected:
  // Protected constructor and destructor
  RecordTableTrait() = default;
  ~RecordTableTrait() = default;

  // CRTP helpers
  Derived& derived() noexcept {
    return static_cast<Derived&>(*this);
  }

  const Derived& derived() const noexcept {
    return static_cast<const Derived&>(*this);
  }
};

}  // namespace PawnDB

#endif
