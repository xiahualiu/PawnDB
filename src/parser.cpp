#include "pawndb/types/parser.h"

namespace PawnDB {

buf_parser::buf_parser(buf_ref& buffer, std::size_t size) noexcept
    : buffer_ref_(buffer), buffer_size_(size) {}

Result<OpType, ParserError> buf_parser::get_op() const noexcept {
  op_t raw;
  if (!parse_field(raw, FieldOffset::OP)) {
    return ParserError::ReadAfterEnd;
  }
  if (raw >= static_cast<op_t>(OpType::MAX_OP_VALUE)) {
    return ParserError::InvalidValue;
  }
  return static_cast<OpType>(raw);
}

Result<op_t, ParserError> buf_parser::get_op_id() const noexcept {
  op_t raw;
  if (!parse_field(raw, FieldOffset::OP_ID)) {
    return ParserError::ReadAfterEnd;
  }
  return raw;
}

Result<txn_id_t, ParserError> buf_parser::get_txn() const noexcept {
  txn_id_t raw;
  if (!parse_field(raw, FieldOffset::TXN)) {
    return ParserError::ReadAfterEnd;
  }
  return raw;
}

Result<tp_id_t, ParserError> buf_parser::get_tbl() const noexcept {
  tp_id_t raw;
  if (!parse_field(raw, FieldOffset::TBL)) {
    return ParserError::ReadAfterEnd;
  }
  return raw;
}

Result<tbl_row_t, ParserError> buf_parser::get_key() const noexcept {
  tbl_row_t raw;
  if (!parse_field(raw, FieldOffset::TP_KEY)) {
    return ParserError::ReadAfterEnd;
  }
  return raw;
}

Result<OpAck, ParserError> buf_parser::get_ack() const noexcept {
  std::uint8_t raw;
  if (!parse_field(raw, FieldOffset::ACK)) {
    return ParserError::ReadAfterEnd;
  }
  if (raw > static_cast<std::uint8_t>(OpAck::DEAD_TXN)) {
    return ParserError::InvalidValue;
  }
  return static_cast<OpAck>(raw);
}

std::string buf_parser::get_string() const noexcept {
  auto& buf = buffer_ref_.buffer();
  std::uint16_t length;
  const auto tup_off = static_cast<std::size_t>(FieldOffset::TUPLE);
  if (tup_off + sizeof(length) > buffer_size_) {
    return {};
  }
  std::memcpy(&length, buf.data() + tup_off, sizeof(length));
  if (tup_off + sizeof(length) + length > buffer_size_) {
    return {};
  }
  return std::string(buf.data() + tup_off + sizeof(length), length);
}

cksum_t buf_parser::get_checksum() const noexcept {
  if (buffer_size_ <= static_cast<std::size_t>(FieldOffset::TUPLE)) {
    return 0;
  }
  cksum_t sum = 0;
  auto& buf = buffer_ref_.buffer();
  for (std::size_t i = 0; i < buffer_size_; i++) {
    sum += static_cast<unsigned char>(buf[i]);
  }
  return sum;
}

void buf_parser::set_ack(OpAck ack) noexcept {
  set_field(ack, FieldOffset::ACK);
}

void buf_parser::set_txn(txn_id_t txn) noexcept {
  set_field(txn, FieldOffset::TXN);
}

void buf_parser::set_key(tbl_row_t key) noexcept {
  set_field(key, FieldOffset::TP_KEY);
}

void buf_parser::set_buffer_size(std::size_t size) noexcept {
  buffer_size_ = size;
}

std::size_t buf_parser::get_buffer_size() const noexcept {
  return buffer_size_;
}

std::size_t buf_parser::get_tuple_offset() const noexcept {
  return static_cast<std::size_t>(FieldOffset::TUPLE);
}

}  // namespace PawnDB
