/**
 * @file lock_key.h
 * @brief CRTP interface for composite lock keys
 * @version 0.1
 * @date 2025-01-02
 *
 * Features:
 * - Key assembly/disassembly
 * - Hash computation
 * - Equality comparison
 */
#ifndef PAWNDB_TRAITS_LOCK_KEY_H
#define PAWNDB_TRAITS_LOCK_KEY_H

#include <cstddef>
#include <utility>

#include "pawndb/params.h"

namespace PawnDB {

/**
 * @brief CRTP interface for lock key implementations
 * @tparam Derived Class implementing key interface
 *
 * Required implementations:
 * - void trait_assemble(tbl_row_t, tbl_row_t)
 * - std::pair<tbl_row_t,tbl_row_t> trait_disassemble() const
 * - std::size_t trait_hash() const
 * - bool trait_equals(const Derived&) const
 */
template <typename Derived>
class CompositeKeyTrait {
 public:
  /**
   * @brief Assemble key from components
   * @param table_id Table identifier
   * @param tuple_key Tuple identifier
   */
  void assemble(tbl_row_t table_id, tbl_row_t tuple_key) noexcept {
    return static_cast<Derived*>(this)->trait_assemble(table_id, tuple_key);
  }

  /**
   * @brief Disassemble key into components
   * @return Pair of {table_id, tuple_key}
   */
  std::pair<tbl_row_t, tbl_row_t> disassemble() const noexcept {
    return static_cast<const Derived*>(this)->trait_disassemble();
  }

  /**
   * @brief Compute hash value for key
   * @return Hash value
   */
  std::size_t hash() const noexcept {
    return static_cast<const Derived*>(this)->trait_hash();
  }

  /**
   * @brief Compare keys for equality
   * @param other Key to compare
   * @return True if keys are equal
   */
  bool operator==(const Derived& other) const noexcept {
    return static_cast<const Derived*>(this)->trait_equals(other);
  }

  /**
   * @brief Compare keys for inequality
   * @param other Key to compare
   * @return True if keys are not equal
   */
  bool operator!=(const Derived& other) const noexcept {
    return !static_cast<const Derived*>(this)->trait_equals(other);
  }

 protected:
  CompositeKeyTrait() = default;
  ~CompositeKeyTrait() = default;
};

}  // namespace PawnDB

#endif  // PAWNDB_TRAITS_KEY_H
