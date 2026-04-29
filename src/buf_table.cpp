#include "pawndb/types/buf_table.h"

namespace PawnDB {

buf_table::buf_req_r buf_table::request() noexcept {
  std::size_t index = 0;
  {
    std::lock_guard<std::mutex> lock(mutex_);
    if (size_ >= BUFFER_ROWS) {
      return BufferError::Full;
    }
    while (buffers_[next_].ref_count_ > 0) {
      next_ = (next_ + 1) % BUFFER_ROWS;
    }
    index = next_;
    buffers_[index].ref_count_ = 1;
    next_ = (index + 1) % BUFFER_ROWS;
    size_++;
  }
  return buf_ref{this, index};
}

bool buf_table::empty() const noexcept {
  std::lock_guard<std::mutex> lock(mutex_);
  return size_ == 0;
}

bool buf_table::full() const noexcept {
  std::lock_guard<std::mutex> lock(mutex_);
  return size_ >= BUFFER_ROWS;
}

std::size_t buf_table::size() const noexcept {
  std::lock_guard<std::mutex> lock(mutex_);
  return size_;
}

void buf_table::retain_ref(std::size_t index) noexcept {
  std::lock_guard<std::mutex> lock(mutex_);
  auto &entry = buffers_[index];
  if (entry.ref_count_ == 0) {
    size_++;
  }
  entry.ref_count_++;
}

void buf_table::release_ref(std::size_t index) noexcept {
  std::lock_guard<std::mutex> lock(mutex_);
  auto &entry = buffers_[index];
  if (entry.ref_count_ == 0) {
    return;
  }
  entry.ref_count_--;
  if (entry.ref_count_ == 0 && size_ > 0) {
    size_--;
  }
}

}  // namespace PawnDB
