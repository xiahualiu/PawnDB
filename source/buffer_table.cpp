#include "pawndb/types/buffer_table.h"

namespace PawnDB {

BufferTable::RequestR BufferTable::trait_request() noexcept {
  if (trait_full()) {
    return BufferError::Full;
  }
  std::lock_guard<std::mutex> lock(mutex_);
  while (buffers_[next_].is_used) {
    next_ = (next_ + 1) % N;
  }
  buffers_[next_].is_used = true;
  size_++;
  return next_++;
}

BufferTable::RequestR BufferTable::trait_release(std::size_t index) noexcept {
  if (index >= N) return BufferError::OutOfRange;
  std::lock_guard<std::mutex> lock(mutex_);
  if (buffers_[index].is_used) {
    buffers_[index].is_used = false;
    size_--;
    return BufferError::None;
  } else {
    return BufferError::NotUsed;
  }
}

buffer_t& BufferTable::trait_get(std::size_t index) noexcept {
  return buffers_[index].buffer;
}

}  // namespace PawnDB
