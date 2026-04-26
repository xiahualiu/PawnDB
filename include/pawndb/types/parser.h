/**
 * @file parser.h
 * @author Xiahua Liu @xiahualiu
 * @brief PawnDB binary protocol parser.
 * @version 0.1
 * @date 2025-01-02
 *
 * @copyright MIT License
 *
 */

#ifndef PAWNDB_TYPES_PARSER_H
#define PAWNDB_TYPES_PARSER_H

#include <array>
#include <cstddef>
#include <cstdint>

#include "pawndb/params.h"

namespace PawnDB {

/**
 * @brief Binary parser for decoding protocol messages in pawnDB.
 *
 * Input format:
 * | txn_id ( 4 bytes ) | op ( 1 byte ) | table_id ( 1 byte ) | tp_key
 * ( 1 byte ) | length ( 2 bytes ) | data (length bytes) |
 *
 * Output format:
 * | txn_id ( 4 bytes ) | op_ack ( 1 byte ) | length ( 2 bytes ) | data (length
 * bytes) |
 */
class Parser {
 public:
  /**
   * @brief Initialize parser with a buffer reference.
   *
   * @param buffer Buffer containing the message to parse.
   * @param size Size of the message in the buffer.
   */
  Parser(const buffer_t& buffer, std::size_t size) noexcept;

  /** @brief Get the transaction ID */
  txn_id_r get_transaction_id() const noexcept;

  /** @brief Get the operation ID */
  op_id_r get_operation_id() const noexcept;

  /** @brief Get the operation type */
  op_r get_operation() const noexcept;

  /** @brief Get the table identifier */
  tbl_id_r get_table_id() const noexcept;

  /** @brief Get the tuple key */
  tp_key_r get_tuple_key() const noexcept;

  /** @brief Get the acknowledgment */
  op_ack_r get_ack() const noexcept;

  /** @brief Get a string field */
  std::string get_string() const noexcept;

  /** @brief Get the checksum */
  cksum_t get_checksum() const noexcept;

  /** @brief Parse a header field into the given reference */
  template <typename T>
  bool parseField(T& value, std::size_t offset) const noexcept;

 private:
  const buffer_t& buffer_ref_; /**< Reference to the message buffer */
  std::size_t size_;           /**< Size of the message */
};

// ============================================================================
// Template Implementation
// ============================================================================

template <typename T>
bool Parser::parseField(T& value, std::size_t offset) const noexcept {
  if (offset + sizeof(T) > size_) {
    return false;
  }
  std::memcpy(&value, buffer_ref_.data() + offset, sizeof(T));
  return true;
}

}  // namespace PawnDB

#endif  // PAWNDB_TYPES_PARSER_H