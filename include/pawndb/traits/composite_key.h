#ifndef PAWNDB_TRAITS_COMPOSITE_KEY_H
#define PAWNDB_TRAITS_COMPOSITE_KEY_H

#include <utility>

#include "pawndb/params.h"

namespace PawnDB {

/**
 * @brief CRTP base class for composite key implementation
 * @tparam Derived The derived key class implementing required traits
 *
 * Combines table ID and tuple key into a single entity for efficient lookup and
 * storage.
 *
 * Required trait implementations:
 * - trait_assemble(table_key_t, tuple_key_t) -> void
 * - trait_disassemble() -> table_key_t, tuple_key_t
 */
template <typename Derived>
class CompositeKeyTrait {
 public:
  /** @brief Table identifier type */
  using table_key_t = tp_id_t;

  /** @brief Table identifier type */
  using tuple_key_t = tbl_row_t;

  /**
   * @brief Assemble key from components
   * @param _table_id Table identifier
   * @param _tuple_key Tuple identifier
   * @post Key contains combined table_id and tuple_key
   */
  void assemble(table_key_t _table_id, tuple_key_t _tuple_key) noexcept {
    return static_cast<Derived*>(this)->trait_assemble(_table_id, _tuple_key);
  }

  /**
   * @brief Disassemble key into table and tuple identifiers
   * @return Pair of {table_id, tuple_key}
   */
  std::pair<table_key_t, tuple_key_t> disassemble() const noexcept {
    return static_cast<const Derived*>(this)->trait_disassemble();
  }

 protected:
  CompositeKeyTrait() = default;
  ~CompositeKeyTrait() = default;
};

}  // namespace PawnDB

#endif  // PAWNDB_TRAITS_KEY_H
