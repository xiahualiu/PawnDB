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

class Parser {
 public:
  Parser(BufferTable::buffer_t& _data, buf_size_t _buffer_size)
      : data(_data), buffer_size(_buffer_size) {}

  Result<OpType, ParserError> get_op() const noexcept {
    if (buffer_size < 1)
      return ParserError::Bad;
    else if (static_cast<OpType>(data[0]) >= OpType::MAX_OP_VALUE)
      return ParserError::Bad;
    return static_cast<OpType>(data[0]);
  }
  Result<op_t, ParserError> get_op_id() const noexcept {
    if (buffer_size < 2)
      return ParserError::Bad;
    else
      return static_cast<op_t>(data[1]);
  }
  Result<txn_id_t, ParserError> get_txn() const noexcept {
    if (buffer_size < 7)
      return ParserError::Bad;
    else
      return *reinterpret_cast<txn_id_t*>(&data[3]);
  }
  Result<tp_id_t, ParserError> get_tbl() const noexcept {
    if (buffer_size < 8)
      return ParserError::Bad;
    else
      return static_cast<tp_id_t>(static_cast<tp_id_t>(data[7]));
  }

  Result<tbl_row_t, ParserError> get_tp_key() const noexcept {
    if (buffer_size < 9)
      return ParserError::Bad;
    else
      return static_cast<tbl_row_t>(data[8]);
  }

  Result<buf_size_t, ParserError> get_data_size() const noexcept {
    if (buffer_size < 13)
      return ParserError::Bad;
    else
      return *reinterpret_cast<buf_size_t*>(&data[9]);
  }

  constexpr buf_size_t get_data_offset() const noexcept { return 13; }

  buf_size_t get_buffer_size() const noexcept { return buffer_size; }

  void set_ack(OpAck ack) noexcept { data[2] = static_cast<char>(ack); }

  void set_txn_id(txn_id_t txn) noexcept {
    *reinterpret_cast<txn_id_t*>(&data[3]) = txn;
  }

  void set_tp_key(tbl_row_t tp) noexcept { data[8] = static_cast<char>(tp); }

  void set_data_size(buf_size_t size) noexcept {
    *reinterpret_cast<buf_size_t*>(&data[9]) = size;
  }

  void set_buffer_size(buf_size_t size) noexcept { buffer_size = size; }

  BufferTable::buffer_t& buffer() noexcept { return data; }

 private:
  BufferTable::buffer_t& data;
  buf_size_t buffer_size;
};

}  // namespace PawnDB

#endif
