#include "pawndb/types/commit_table.h"

namespace PawnDB {

CommitEntry::CommitEntry(const TableTupleKey& _key, OpType _op,
                         const BufferRef& _buffer) noexcept
    : buffer_(_buffer), key_(_key), op_(_op) {}

CommitEntry::CommitEntry(const CommitEntry& other) noexcept
    : buffer_(other.buffer_), key_(other.key_), op_(other.op_) {}

CommitEntry& CommitEntry::operator=(const CommitEntry& other) noexcept {
  buffer_ = other.buffer_;
  key_ = other.key_;
  op_ = other.op_;
  return *this;
}

CommitEntry CommitEntry::copy() const noexcept {
  return CommitEntry(*this);
}

void CommitEntry::copy_from(const CommitEntry& other) noexcept {
  *this = other;
}

BufferRef CommitEntry::buffer() const noexcept {
  return buffer_;
}

const TableTupleKey& CommitEntry::key() const noexcept {
  return key_;
}

OpType CommitEntry::op() const noexcept {
  return op_;
}

CommitTable::queue_r CommitTable::get() noexcept {
  return commits_[tail_];
}

QueueError CommitTable::send(const CommitEntry& _entry) noexcept {
  if (full()) {
    return QueueError::Full;
  }
  commits_[head_] = _entry;
  head_ = (head_ + 1) % N;
  size_++;
  return QueueError::None;
}

void CommitTable::pop() noexcept {
  tail_ = (tail_ + 1) % N;
  size_--;
}

void CommitTable::clear() noexcept {
  head_ = 0;
  tail_ = 0;
  size_ = 0;
}

std::size_t CommitTable::size() const noexcept {
  return size_;
}

bool CommitTable::empty() const noexcept {
  return size_ == 0;
}

bool CommitTable::full() const noexcept {
  return size_ >= N;
}

}  // namespace PawnDB