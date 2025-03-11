#include "pawndb/types/rollback_deque.h"

#include "pawndb/traits/deque.h"

namespace PawnDB {

RollbackEntry::RollbackEntry(const TableTupleKey& key, OpType op,
                             const BufferRef& buffer) noexcept
    : buffer_(buffer), key_(key), op_(op) {}

RollbackEntry::RollbackEntry(const RollbackEntry& other) noexcept
    : buffer_(other.buffer_), key_(other.key_), op_(other.op_) {}

RollbackEntry& RollbackEntry::operator=(const RollbackEntry& other) noexcept {
  buffer_ = other.buffer_;
  key_ = other.key_;
  op_ = other.op_;
  return *this;
}

BufferRef RollbackEntry::trait_buf() const noexcept {
  return buffer_;
}

TableTupleKey RollbackEntry::trait_key() const noexcept {
  return key_;
}

OpType RollbackEntry::trait_op() const noexcept {
  return op_;
}

DequeError RollbackDeque::trait_push_back(const RollbackEntry& entry) noexcept {
  if (trait_full()) {
    return DequeError::Full;
  }
  rollbacks_[tail_] = entry;
  tail_ = (tail_ + 1) % N;
  ++size_;
  return DequeError::None;
}

RollbackDeque::deque_r RollbackDeque::trait_front() noexcept {
  if (trait_empty()) {
    return DequeError::Empty;
  }
  return rollbacks_[head_];
}

RollbackDeque::deque_r RollbackDeque::trait_back() noexcept {
  if (trait_empty()) {
    return DequeError::Empty;
  }
  return rollbacks_[(tail_ - 1) % N];
}

DequeError RollbackDeque::trait_pop_front() noexcept {
  if (trait_empty()) {
    return DequeError::Empty;
  }
  head_ = (head_ + 1) % N;
  --size_;
  return DequeError::None;
}

DequeError RollbackDeque::trait_pop_back() noexcept {
  if (trait_empty()) {
    return DequeError::Empty;
  }
  tail_ = (tail_ - 1) % N;
  --size_;
  return DequeError::None;
}

std::size_t RollbackDeque::trait_size() const noexcept {
  return size_;
}

bool RollbackDeque::trait_full() const noexcept {
  return size_ == N;
}

bool RollbackDeque::trait_empty() const noexcept {
  return size_ == 0;
}

void RollbackDeque::trait_clear() noexcept {
  head_ = 0;
  tail_ = 0;
  size_ = 0;
}

}  // namespace PawnDB
