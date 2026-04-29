#include "pawndb/types/commit.h"

namespace PawnDB {

commit::commit(const unique_key& _key, OpType _op,
               const buf_ref& _buffer) noexcept
    : buffer_(_buffer), key_(_key), op_(_op) {}

commit::commit(const commit& other) noexcept
    : buffer_(other.buffer_), key_(other.key_), op_(other.op_) {}

commit& commit::operator=(const commit& other) noexcept {
  buffer_ = other.buffer_;
  key_ = other.key_;
  op_ = other.op_;
  return *this;
}

buf_ref commit::buf() const noexcept {
  return buffer_;
}

const unique_key& commit::key() const noexcept {
  return key_;
}

OpType commit::op() const noexcept {
  return op_;
}

}  // namespace PawnDB
