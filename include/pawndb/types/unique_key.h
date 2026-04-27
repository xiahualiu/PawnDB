#ifndef PAWNDB_TYPES_UNIQUE_KEY_H
#define PAWNDB_TYPES_UNIQUE_KEY_H

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
class unique_key {
 public:
  /** @brief Default constructor */
  constexpr unique_key() noexcept : table_id_(0), tuple_key_(0) {}

  /**
   * @brief Construct key from components
   * @param table_id Table identifier
   * @param tuple_key Tuple key
   */
  unique_key(tp_id_t table_id, tbl_row_t tuple_key) noexcept;

  // Copyable
  unique_key(const unique_key& other) noexcept;
  unique_key& operator=(const unique_key& other) noexcept;

  // Movable
  unique_key(unique_key&& other) noexcept;
  unique_key& operator=(unique_key&& other) noexcept;

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
  bool equals_(const unique_key& other) const noexcept;

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

#endif  // PAWNDB_TYPES_UNIQUE_KEY_H
