#include "pawndb/types/rollback.h"

namespace PawnDB {

rollback::rollback(const unique_key& _key, OpType _op,
                   const buf_ref& _buffer) noexcept
    : buffer_(_buffer), key_(_key), op_(_op) {}

rollback::rollback(const rollback& other) noexcept
    : buffer_(other.buffer_), key_(other.key_), op_(other.op_) {}

rollback& rollback::operator=(const rollback& other) noexcept {
  buffer_ = other.buffer_;
  key_ = other.key_;
  op_ = other.op_;
  return *this;
}

buf_ref rollback::buf() const noexcept {
  return buffer_;
}

const unique_key& rollback::key() const noexcept {
  return key_;
}

OpType rollback::op() const noexcept {
  return op_;
}

}  // namespace PawnDB
