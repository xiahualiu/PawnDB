#include "pawndb/types/commit_table.h"

namespace PawnDB {

commit_entry::commit_entry(const unique_key& _key, OpType _op,
                           const buf_ref& _buffer) noexcept
    : buffer_(_buffer), key_(_key), op_(_op) {}

commit_entry::commit_entry(const commit_entry& other) noexcept
    : buffer_(other.buffer_), key_(other.key_), op_(other.op_) {}

commit_entry& commit_entry::operator=(const commit_entry& other) noexcept {
  buffer_ = other.buffer_;
  key_ = other.key_;
  op_ = other.op_;
  return *this;
}

buf_ref commit_entry::buf() const noexcept {
  return buffer_;
}

const unique_key& commit_entry::key() const noexcept {
  return key_;
}

OpType commit_entry::op() const noexcept {
  return op_;
}

commit_table::queue_r commit_table::get() noexcept {
  return commits_[tail_];
}

QueueError commit_table::send(const commit_entry& _entry) noexcept {
  if (full()) {
    return QueueError::Full;
  }
  commits_[head_] = _entry;
  head_ = (head_ + 1) % N;
  size_++;
  return QueueError::None;
}

void commit_table::pop() noexcept {
  tail_ = (tail_ + 1) % N;
  size_--;
}

void commit_table::clear() noexcept {
  head_ = 0;
  tail_ = 0;
  size_ = 0;
}

std::size_t commit_table::size() const noexcept {
  return size_;
}

bool commit_table::empty() const noexcept {
  return size_ == 0;
}

bool commit_table::full() const noexcept {
  return size_ >= N;
}

}  // namespace PawnDB
