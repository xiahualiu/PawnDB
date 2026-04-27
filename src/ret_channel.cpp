#include "pawndb/types/ret_channel.h"

#include <cstddef>

namespace PawnDB {

ret_channel::ret_channel() noexcept : rets_(), head_(0), tail_(0), count_(0) {}

ret_channel::queue_r ret_channel::get() noexcept {
  std::unique_lock<std::mutex> lock(mtx_);
  if (!empty()) {
    return rets_[head_];
  } else {
    return QueueError::Empty;
  }
}

QueueError ret_channel::send(const ret& _ret) noexcept {
  std::unique_lock<std::mutex> lock(mtx_);
  rets_[tail_] = _ret;
  tail_ = (tail_ + 1) % MaxRets;
  count_++;
  return QueueError::None;
}

void ret_channel::clear() noexcept {
  std::unique_lock<std::mutex> lock(mtx_);
  head_ = 0;
  tail_ = 0;
  count_ = 0;
}

void ret_channel::pop() noexcept {
  std::unique_lock<std::mutex> lock(mtx_);
  head_ = (head_ + 1) % MaxRets;
  count_--;
}

std::size_t ret_channel::size() const noexcept {
  return count_;
}

bool ret_channel::empty() const noexcept {
  return count_ == 0;
}

bool ret_channel::full() const noexcept {
  return count_ == MaxRets;
}

}  // namespace PawnDB
