#ifndef PAWNDB_TYPES_TABLE_TUPLE_KEY_H
#define PAWNDB_TYPES_TABLE_TUPLE_KEY_H

#include <cstdint>
#include <cstring>

#include "pawndb/params.h"
#include "pawndb/traits/composite_key.h"
#include "pawndb/traits/copy.h"
#include "pawndb/traits/eq.h"
#include "pawndb/traits/hash.h"
#include "pawndb/traits/move.h"

namespace PawnDB {

/**
 * @brief Composite key combining table ID and tuple key
 *
 * Combines table identifier and tuple key into a single entity.
 * Implements composite key, copy, and hash operations.
 */
class TableTupleKey : public CompositeKeyTrait<TableTupleKey>,
                      public CopyTrait<TableTupleKey>,
                      public MoveTrait<TableTupleKey>,
                      public HashTrait<TableTupleKey>,
                      public EqTrait<TableTupleKey> {
 public:
  /** @brief Default constructor */
  constexpr TableTupleKey() noexcept : table_id_(0), tuple_key_(0) {}

  /**
   * @brief Construct key from components
   * @param table_id Table identifier
   * @param tuple_key Tuple key
   */
  TableTupleKey(tp_id_t table_id, tbl_row_t tuple_key) noexcept;

  // Copyable
  TableTupleKey(const TableTupleKey& other) noexcept;
  TableTupleKey& operator=(const TableTupleKey& other);

  // CompositeKeyTrait implementation
  /**
   * @brief Assemble key from components
   * @param table_id Table identifier
   * @param tuple_key Tuple key
   */
  void trait_assemble(std::uint8_t table_id, std::uint8_t tuple_key) noexcept;

  /**
   * @brief Disassemble key into components
   * @return Pair of {table_id, tuple_key}
   */
  std::pair<std::uint8_t, std::uint8_t> trait_disassemble() const noexcept;

  // HashTrait implementation
  /** @brief Compute hash value */
  std::size_t trait_hash() const noexcept;

  /** @brief Compare for equality
   * @param other Key to compare with
   * @return true if keys are equal */
  bool trait_equals(const TableTupleKey& other) const noexcept;

  // CopyTrait implementation
  /** @brief Create a value copy of this key */
  TableTupleKey trait_copy() const noexcept;

  /** @brief Copy from another key
   * @param other Source key to copy from */
  void trait_copy_from(const TableTupleKey& other) noexcept;
  // MoveTrait implementation
  /** @brief Move out a new key from this one */
  TableTupleKey trait_move() noexcept;

  /** @brief Move-assign from another key */
  void trait_move_from(TableTupleKey&& other) noexcept;

 private:
  std::uint8_t table_id_;
  std::uint8_t tuple_key_;
};

}  // namespace PawnDB

#endif  // PAWNDB_TYPES_SIMPLE_KEY_H
