#ifndef PAWNDB_TRAITS_TUPLE_H
#define PAWNDB_TRAITS_TUPLE_H

#include <ctime>

#include "pawndb/params.h"

namespace PawnDB {

/**
 * @brief CRTP interface for tuple implementations
 * @tparam Derived Class implementing tuple interface
 *
 * Required implementations:
 * - void trait_set_checksum()
 * - bool trait_val_checksum() const
 * - void trait_set_tickstamp(const tick_t)
 * - tick_t trait_read_tickstamp() const
 */
template <typename Derived>
class TupleTrait {
 public:
  /**
   * @brief Set tuple checksum
   */
  void set_checksum() noexcept {
    static_cast<Derived*>(this)->trait_set_checksum();
  }

  /**
   * @brief Validate tuple checksum
   * @return true if checksum valid
   */
  bool val_checksum() const noexcept {
    return static_cast<const Derived*>(this)->trait_val_checksum();
  }

  /**
   * @brief Set tuple timestamp
   * @param _ts Timestamp value
   */
  void set_tickstamp(const tick_t _ts) noexcept {
    static_cast<Derived*>(this)->trait_set_tickstamp(_ts);
  }

  /**
   * @brief Read tuple timestamp
   * @return Current timestamp
   */
  tick_t read_tickstamp() const noexcept {
    return static_cast<const Derived*>(this)->trait_read_tickstamp();
  }

  tbl_row_t key() const noexcept {
    return static_cast<const Derived*>(this)->trait_key();
  }

 protected:
  // Protected constructor and destructor
  TupleTrait() = default;
  ~TupleTrait() = default;
};

}  // namespace PawnDB

#endif  // PAWNDB_TRAITS_TUPLE_H
