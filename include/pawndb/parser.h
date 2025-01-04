/**
 * @file parser.h
 * @author Xiahua Liu @xiahualiu
 * @brief PawnDB parser class, used for parsing buffer data.
 * @version 0.1
 * @date 2025-01-02
 *
 * @copyright MIT License
 *
 */

#ifndef PAWNDB_PARSER_H
#define PAWNDB_PARSER_H

#include <cstdint>
#include <cstring>

#include "pawndb/buffer_table.h"
#include "pawndb/params.h"
#include "pawndb/result.h"

namespace PawnDB {

enum class OpType : std::uint8_t {
  START_TXN = 1,
  COMMIT_TXN,
  ABORT_TXN,
  ADD_TUPLE,
  SHARED_READ,
  EXCLUSIVE_READ,
  YIELD_READ,
  PROMOTE,
  UPDATE,
  DELETE,
  MAX_OP_VALUE,
};

enum class OpAck : std::uint8_t {
  SUCCESS = 1,
  DEAD_TXN,
  BAD_OP,
  BAD_TXN,
  BAD_TABLE,
  BAD_TP,
  BAD_ACCESS,
  BAD_PHASE,
  BAD_DATA,
  COMMIT_FULL,
  ABORTED,
  TIMEOUT,
  BUSY,
};

enum class ParserError { None, Bad };

/**
 * @brief Class representing a parser for operations.
 */
class Parser {
 public:
  Parser(BufferTable::buffer_t& _data, buf_size_t _buffer_size)
      : data(_data), buffer_size(_buffer_size) {}

  /**
   * @brief Gets the operation type.
   *
   * @return The operation type.
   */
  Result<OpType, ParserError> get_op() const noexcept {
    if (buffer_size < 1)
      return ParserError::Bad;
    else if (static_cast<OpType>(data[0]) >= OpType::MAX_OP_VALUE)
      return ParserError::Bad;
    return static_cast<OpType>(data[0]);
  }

  /**
   * @brief Gets the operation ID.
   *
   * @return The operation ID.
   */
  Result<op_t, ParserError> get_op_id() const noexcept {
    if (buffer_size < 2)
      return ParserError::Bad;
    else
      return static_cast<op_t>(data[1]);
  }

  /**
   * @brief Gets the transaction id
   *
   * @return The transaction id.
   */
  Result<txn_id_t, ParserError> get_txn() const noexcept {
    if (buffer_size < 7)
      return ParserError::Bad;
    else
      return *reinterpret_cast<txn_id_t*>(&data[3]);
  }

  /**
   * @brief Gets the table ID.
   *
   * @return The table ID.
   */
  Result<tp_id_t, ParserError> get_tbl() const noexcept {
    if (buffer_size < 8)
      return ParserError::Bad;
    else
      return static_cast<tp_id_t>(static_cast<tp_id_t>(data[7]));
  }

  /**
   * @brief Gets the tuple key.
   *
   * @return The tuple key.
   */
  Result<tbl_row_t, ParserError> get_tp_key() const noexcept {
    if (buffer_size < 9)
      return ParserError::Bad;
    else
      return static_cast<tbl_row_t>(data[8]);
  }

  /**
   * @brief Gets the data size.
   *
   * @return The data size.
   */
  Result<buf_size_t, ParserError> get_data_size() const noexcept {
    if (buffer_size < 13)
      return ParserError::Bad;
    else
      return *reinterpret_cast<buf_size_t*>(&data[9]);
  }

  /**
   * @brief Gets the data field offset.
   *
   * @return The data field offset.
   */
  constexpr buf_size_t get_data_offset() const noexcept { return 13; }

  /**
   * @brief Gets the buffer size.
   *
   * @return The buffer size.
   */
  buf_size_t get_buffer_size() const noexcept { return buffer_size; }

  /**
   * @brief Sets the acknowledgment operation.
   *
   * @param ack The acknowledgment operation to set.
   */
  void set_ack(OpAck ack) noexcept { data[2] = static_cast<char>(ack); }

  /**
   * @brief Sets the transaction ID.
   *
   * @param txn The transaction ID to set.
   */
  void set_txn_id(txn_id_t txn) noexcept {
    *reinterpret_cast<txn_id_t*>(&data[3]) = txn;
  }

  /**
   * @brief Sets the tuple key.
   *
   * @param tp The tuple key to set.
   */
  void set_tp_key(tbl_row_t tp) noexcept { data[8] = static_cast<char>(tp); }

  /**
   * @brief Sets the data size.
   *
   * @param size The data size to set.
   */
  void set_data_size(buf_size_t size) noexcept {
    *reinterpret_cast<buf_size_t*>(&data[9]) = size;
  }

  /**
   * @brief Sets the buffer size.
   *
   * @param size The buffer size to set.
   */
  void set_buffer_size(buf_size_t size) noexcept { buffer_size = size; }

  /**
   * @brief Gets the buffer.
   *
   * @return A reference to the buffer.
   */
  BufferTable::buffer_t& buffer() noexcept { return data; }

 private:
  BufferTable::buffer_t& data;
  buf_size_t buffer_size;
};

}  // namespace PawnDB

#endif
