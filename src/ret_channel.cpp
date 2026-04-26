#include "pawndb/types/ret_channel.h"

#include <cstddef>

namespace PawnDB {

RetChannel::RetChannel() noexcept : rets_(), head_(0), tail_(0), count_(0) {}

RetChannel::queue_r RetChannel::get() noexcept {
  std::unique_lock<std::mutex> lock(mtx_);
  if (!empty()) {
    return rets_[head_];
  } else {
    return QueueError::Empty;
  }
}

QueueError RetChannel::send(const Ret& ret) noexcept {
  std::unique_lock<std::mutex> lock(mtx_);
  rets_[tail_] = ret;
  tail_ = (tail_ + 1) % MaxRets;
  count_++;
  return QueueError::None;
}

void RetChannel::clear() noexcept {
  std::unique_lock<std::mutex> lock(mtx_);
  head_ = 0;
  tail_ = 0;
  count_ = 0;
}

void RetChannel::pop() noexcept {
  std::unique_lock<std::mutex> lock(mtx_);
  head_ = (head_ + 1) % MaxRets;
  count_--;
}

std::size_t RetChannel::size() const noexcept {
  return count_;
}

bool RetChannel::empty() const noexcept {
  return count_ == 0;
}

bool RetChannel::full() const noexcept {
  return count_ == MaxRets;
}

}  // namespace PawnDB