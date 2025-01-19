#ifndef PAWNDB_TYPES_PARSER_H
#define PAWNDB_TYPES_PARSER_H

#include <cstddef>
#include <cstring>

#include "pawndb/params.h"
#include "pawndb/traits/parser.h"
#include "pawndb/types/buffer_table.h"

namespace PawnDB {

class Parser : public ParserTrait<Parser> {
 public:
  Parser(const BufferRef& _buffers, std::size_t _size) noexcept
      : buffers_(_buffers.buffer()), size_(_size) {}

  Parser(buffer_t& _buffer, std::size_t _size) noexcept
      : buffers_(_buffer), size_(_size) {}

  // Non-copyable
  Parser(const Parser& _other) = delete;
  Parser& operator=(const Parser& _other) = delete;

  op_r trait_get_op() const noexcept {
    if (size_ < 1) return ParserError::ReadAfterEnd;
    auto op = static_cast<OpType>(buffers_[0]);
    if (op >= OpType::MAX_OP_VALUE) return ParserError::InvalidValue;
    return op;
  }

  op_id_r trait_get_op_id() const noexcept {
    if (size_ < 2) return ParserError::ReadAfterEnd;
    return static_cast<op_t>(buffers_[1]);
  }

  txn_id_r trait_get_txn() const noexcept {
    if (size_ < 6) return ParserError::ReadAfterEnd;
    txn_id_t txn;
    std::memcpy(&txn, &buffers_[2], sizeof(txn_id_t));
    return txn;
  }

  op_ack_r trait_get_ack() const noexcept {
    if (size_ < 7) return ParserError::ReadAfterEnd;
    auto ack = static_cast<OpAck>(buffers_[6]);
    if (ack > OpAck::BUSY) return ParserError::InvalidValue;
    return ack;
  }

  tbl_id_r trait_get_tbl() const noexcept {
    if (size_ < 8) return ParserError::ReadAfterEnd;
    return static_cast<tp_id_t>(buffers_[7]);
  }

  tp_key_r trait_get_key() const noexcept {
    if (size_ < 9) return ParserError::ReadAfterEnd;
    return static_cast<tbl_row_t>(buffers_[8]);
  }

  constexpr std::size_t trait_get_tuple_offset() const noexcept {
    return 9;
  }

  void trait_set_op(OpType _op) noexcept {
    buffers_[0] = static_cast<char>(_op);
  }

  void trait_set_op_id(op_t _op_id) noexcept {
    buffers_[1] = static_cast<char>(_op_id);
  }

  void trait_set_ack(OpAck _ack) noexcept {
    buffers_[6] = static_cast<char>(_ack);
  }

  void trait_set_txn(txn_id_t _txn) noexcept {
    std::memcpy(&buffers_[2], &_txn, sizeof(txn_id_t));
  }

  void trait_set_tbl(tp_id_t _tbl) noexcept {
    buffers_[7] = static_cast<char>(_tbl);
  }

  void trait_set_key(tbl_row_t _key) noexcept {
    buffers_[8] = static_cast<char>(_key);
  }

  void trait_set_buffer_size(std::size_t _size) noexcept {
    size_ = _size;
  }

  std::size_t trait_get_buffer_size() const noexcept {
    return size_;
  }

  buffer_t& trait_get_buffer() noexcept {
    return buffers_;
  }

 private:
  buffer_t& buffers_;
  std::size_t size_;
};
}  // namespace PawnDB

#endif
