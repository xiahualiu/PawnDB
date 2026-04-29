#include "pawndb/types/commit_queue.h"

namespace PawnDB {

commit_queue::queue_r commit_queue::get() noexcept {
  return commits_[tail_];
}

QueueError commit_queue::send(const commit& _entry) noexcept {
  if (full()) {
    return QueueError::Full;
  }
  commits_[head_] = _entry;
  head_ = (head_ + 1) % N;
  size_++;
  return QueueError::None;
}

void commit_queue::pop() noexcept {
  tail_ = (tail_ + 1) % N;
  size_--;
}

void commit_queue::clear() noexcept {
  head_ = 0;
  tail_ = 0;
  size_ = 0;
}

std::size_t commit_queue::size() const noexcept {
  return size_;
}

bool commit_queue::empty() const noexcept {
  return size_ == 0;
}

bool commit_queue::full() const noexcept {
  return size_ >= N;
}

}  // namespace PawnDB
