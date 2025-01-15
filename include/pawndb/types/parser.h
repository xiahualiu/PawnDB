#ifndef PAWNDB_TYPES_PARSER_H
#define PAWNDB_TYPES_PARSER_H

#include <cstddef>

#include "pawndb/params.h"
#include "pawndb/traits/parser.h"
#include "pawndb/types/buffer_table.h"

namespace PawnDB {

class Parser : public ParserTrait<Parser> {
 public:
  Parser(const BufferRef& _buffers, std::size_t _size) noexcept
      : buffers_(_buffers), size_(_size) {}

  // Non-copyable
  Parser(const Parser& _other) = delete;
  Parser& operator=(const Parser& _other) = delete;

  op_r trait_get_op() const noexcept {
    if (size_ < 1) return ParserError::ReadAfterEnd;
    auto op = static_cast<OpType>(buffers_.buffer()[0]);
    if (op >= OpType::MAX_OP_VALUE) return ParserError::InvalidValue;
    return op;
  }

  op_id_r trait_get_op_id() const noexcept {
    if (size_ < 2) return ParserError::ReadAfterEnd;
    return static_cast<op_t>(buffers_.buffer()[1]);
  }

  txn_id_r trait_get_txn() const noexcept {
    if (size_ < 6) return ParserError::ReadAfterEnd;
    return *reinterpret_cast<txn_id_t*>(&buffers_.buffer()[2]);
  }

  tbl_id_r trait_get_tbl() const noexcept {
    if (size_ < 8) return ParserError::ReadAfterEnd;
    return static_cast<tp_id_t>(buffers_.buffer()[7]);
  }

  tp_key_r trait_get_key() const noexcept {
    if (size_ < 9) return ParserError::ReadAfterEnd;
    return static_cast<tbl_row_t>(buffers_.buffer()[8]);
  }

  constexpr std::size_t trait_get_tuple_offset() const noexcept {
    return 9;
  }

  void trait_set_ack(OpAck _ack) noexcept {
    buffers_.buffer()[6] = static_cast<char>(_ack);
  }

  void trait_set_txn_id(txn_id_t _txn) noexcept {
    *reinterpret_cast<txn_id_t*>(&buffers_.buffer()[2]) = _txn;
  }

  void trait_set_key(tbl_row_t _key) noexcept {
    buffers_.buffer()[8] = static_cast<char>(_key);
  }

  void trait_set_buffer_size(std::size_t _size) noexcept {
    size_ = _size;
  }

  std::size_t trait_get_buffer_size() const noexcept {
    return size_;
  }

  buffer_t& trait_get_buffer() noexcept {
    return buffers_.buffer();
  }

 private:
  BufferRef buffers_;
  std::size_t size_;
};
}  // namespace PawnDB

#endif
