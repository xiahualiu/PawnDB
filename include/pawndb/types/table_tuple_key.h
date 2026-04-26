#ifndef PAWNDB_TYPES_TABLE_TUPLE_KEY_H
#define PAWNDB_TYPES_TABLE_TUPLE_KEY_H

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <utility>

#include "pawndb/params.h"

namespace PawnDB {

/**
 * @brief Composite key combining table ID and tuple key
 *
 * Combines table identifier and tuple key into a single entity.
 * Implements hash, copy, move, and equality operations.
 */
class TableTupleKey {
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
  TableTupleKey& operator=(const TableTupleKey& other) noexcept;

  // Movable
  TableTupleKey(TableTupleKey&& other) noexcept;
  TableTupleKey& operator=(TableTupleKey&& other) noexcept;

  /**
   * @brief Assemble key from components
   * @param table_id Table identifier
   * @param tuple_key Tuple key
   */
  void assemble_(std::uint8_t table_id, std::uint8_t tuple_key) noexcept;

  /**
   * @brief Disassemble key into components
   * @return Pair of {table_id, tuple_key}
   */
  std::pair<std::uint8_t, std::uint8_t> disassemble_() const noexcept;

  /** @brief Compute hash value */
  std::size_t hash_() const noexcept;

  /** @brief Compare for equality
   * @param other Key to compare with
   * @return true if keys are equal */
  bool equals_(const TableTupleKey& other) const noexcept;

  /** @brief Create a value copy of this key */
  TableTupleKey copy_() const noexcept;

  /** @brief Copy from another key
   * @param other Source key to copy from */
  void copy_from_(const TableTupleKey& other) noexcept;

  /** @brief Move out a new key from this one */
  TableTupleKey move_() noexcept;

  /** @brief Move-assign from another key */
  void move_from_(TableTupleKey&& other) noexcept;

  /** @brief Get underlying raw data pointer (for hash computation) */
  const char* data_() const noexcept;

  /** @brief Get size of key data */
  static constexpr std::size_t size_() noexcept {
    return 2;
  }

 private:
  std::uint8_t table_id_;
  std::uint8_t tuple_key_;
};

}  // namespace PawnDB

#endif  // PAWNDB_TYPES_TABLE_TUPLE_KEY_H