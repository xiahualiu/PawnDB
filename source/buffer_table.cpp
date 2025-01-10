#include "pawndb/types/buffer_table.h"

namespace PawnDB {

BufferTable::RequestR BufferTable::trait_request() noexcept {
  if (trait_full()) {
    return BufferError::Full;
  }
  std::lock_guard<std::mutex> lock(mutex_);
  while (buffers_[next_].ref_count > 0) {
    next_ = (next_ + 1) % N;
  }
  buffers_[next_].ref_count = 1;
  auto result = BufferRC{this, next_};
  next_ = (next_ + 1) % N;
  size_++;
  return result;
}

}  // namespace PawnDB
