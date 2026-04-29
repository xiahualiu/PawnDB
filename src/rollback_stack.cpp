#include "pawndb/types/rollback_stack.h"

namespace PawnDB {

QueueError rollback_stack::push(const rollback& entry) noexcept {
  if (full()) {
    return QueueError::Full;
  }
  rollbacks_[size_] = entry;
  size_++;
  return QueueError::None;
}

rollback_stack::stack_r rollback_stack::top() noexcept {
  if (empty()) {
    return QueueError::Empty;
  }
  return rollbacks_[size_ - 1];
}

void rollback_stack::pop() noexcept {
  if (empty()) {
    return;
  }
  size_--;
}

void rollback_stack::clear() noexcept {
  size_ = 0;
}

std::size_t rollback_stack::size() const noexcept {
  return size_;
}

bool rollback_stack::empty() const noexcept {
  return size_ == 0;
}

bool rollback_stack::full() const noexcept {
  return size_ >= N;
}

}  // namespace PawnDB
