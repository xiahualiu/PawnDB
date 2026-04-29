/**
 * @file ret_buf.h
 * @author Xiahua Liu @xiahualiu
 * @brief PawnDB return buffer — owns a buffer and provides protocol
 * composition.
 * @version 0.1
 * @date 2025-01-02
 *
 * @copyright MIT License
 *
 */

#ifndef PAWNDB_TYPES_RET_BUF_H
#define PAWNDB_TYPES_RET_BUF_H

#include <cstddef>
#include <cstring>
#include <string>

#include "pawndb/params.h"
#include "pawndb/result.h"
#include "pawndb/types/buf_ref.h"
#include "pawndb/types/job_buf.h"

namespace PawnDB {

/** @brief Return buffer — a buffer reference with protocol composition for
 * responses.
 *
 * Inherits buf_ref for buffer ownership and adds binary protocol field
 * parsing/composition. Tailored for use within the ret class.
 *
 * Output format:
 * | op (1) | op_id (1) | txn_id (4) | ack (1) | table_id (1) | tp_key (1) |
 * data... |
 */
class ret_buf : public buf_ref {
 public:
  /** @brief Default constructor — creates empty return buffer */
  constexpr ret_buf() noexcept : buf_ref(), buffer_size_(0) {}

  /** @brief Construct from an existing buf_ref, taking ownership */
  ret_buf(buf_ref buf, std::size_t size) noexcept;

  /** @brief Get the underlying buffer */
  buf_t& get_buffer() const noexcept {
    return buffer();
  }

  /** @brief Get the operation type */
  Result<OpType, ParserError> get_op() const noexcept;

  /** @brief Get the operation ID */
  Result<op_t, ParserError> get_op_id() const noexcept;

  /** @brief Get the transaction ID */
  Result<txn_id_t, ParserError> get_txn() const noexcept;

  /** @brief Get the table identifier */
  Result<tp_id_t, ParserError> get_tbl() const noexcept;

  /** @brief Get the tuple key */
  Result<tbl_row_t, ParserError> get_key() const noexcept;

  /** @brief Get the acknowledgment */
  Result<OpAck, ParserError> get_ack() const noexcept;

  /** @brief Get a string field from the data section */
  std::string get_string() const noexcept;

  /** @brief Get the checksum */
  cksum_t get_checksum() const noexcept;

  /** @brief Set the operation type */
  void set_op(OpType op) noexcept;

  /** @brief Set the operation ID */
  void set_op_id(op_t id) noexcept;

  /** @brief Set the acknowledgment field */
  void set_ack(OpAck ack) noexcept;

  /** @brief Set the transaction ID field */
  void set_txn(txn_id_t txn) noexcept;

  /** @brief Set the table identifier */
  void set_tbl(tp_id_t tbl) noexcept;

  /** @brief Set the tuple key field */
  void set_key(tbl_row_t key) noexcept;

  /** @brief Set a string into the data section */
  void set_string(const std::string& str) noexcept;

  /** @brief Set the buffer size */
  void set_buffer_size(std::size_t size) noexcept;

  /** @brief Get the buffer size */
  std::size_t get_buffer_size() const noexcept;

  /** @brief Get the offset where tuple data begins */
  std::size_t get_tuple_offset() const noexcept;

 private:
  std::size_t buffer_size_;

  template <typename T>
  bool parse_field(T& value, FieldOffset offset) const noexcept;

  template <typename T>
  void set_field(const T& value, FieldOffset offset) noexcept;
};

template <typename T>
bool ret_buf::parse_field(T& value, FieldOffset offset) const noexcept {
  const auto off = static_cast<std::size_t>(offset);
  if (off + sizeof(T) > buffer_size_) {
    return false;
  }
  std::memcpy(&value, buffer().data() + off, sizeof(T));
  return true;
}

template <typename T>
void ret_buf::set_field(const T& value, FieldOffset offset) noexcept {
  std::memcpy(buffer().data() + static_cast<std::size_t>(offset), &value,
              sizeof(T));
}

}  // namespace PawnDB

#endif  // PAWNDB_TYPES_RET_BUF_H
