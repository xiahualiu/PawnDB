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
 */
template <typename Derived>
class RecordTrait {
 public:
  void set_checksum() noexcept {
    derived().trait_set_checksum();
  }

  bool val_checksum() const noexcept {
    return derived().trait_val_checksum();
  }

  tbl_row_t key() const noexcept {
    return derived().trait_key();
  }

 protected:
  // Protected constructor and destructor
  RecordTrait() = default;
  ~RecordTrait() = default;

  // CRTP helpers
  Derived& derived() noexcept {
    return static_cast<Derived&>(*this);
  }

  const Derived& derived() const noexcept {
    return static_cast<const Derived&>(*this);
  }
};

}  // namespace PawnDB

#endif  // PAWNDB_TRAITS_TUPLE_H
