#include "pawndb/types/buffer_table.h"

namespace PawnDB {

BufferTable::request_r BufferTable::trait_request() noexcept {
  std::lock_guard<std::mutex> lock(mutex_);
  if (trait_full()) {
    return BufferError::Full;
  }
  while (buffers_[next_].is_used_ > 0) {
    next_ = (next_ + 1) % N;
  }
  auto result = BufferRef{this, next_};
  buffers_[next_].is_used_ = true;
  next_ = (next_ + 1) % N;
  size_++;
  return result;
}

void BufferTable::trait_clear() noexcept {
  std::lock_guard<std::mutex> lock(mutex_);
  for (auto &buffer : buffers_) {
    buffer.is_used_ = false;
  }
  size_ = 0;
}

// bool BufferTable::trait_empty() const noexcept {
//   return size_ == 0;
// }

bool BufferTable::trait_full() const noexcept {
  return size_ >= N;
}

std::size_t BufferTable::trait_size() const noexcept {
  return size_;
}

std::uint8_t BufferTable::_test_is_used(std::size_t _i) const noexcept {
  return buffers_[_i].is_used_;
}

BufferRef::BufferRef(BufferTable *_table, std::size_t _index) noexcept
    : table_(_table), index_(_index) {}

BufferRef::BufferRef(const BufferRef &_other) noexcept
    : table_(_other.table_), index_(_other.index_) {}

BufferRef &BufferRef::operator=(const BufferRef &_other) noexcept {
  table_ = _other.table_;
  index_ = _other.index_;
  return *this;
}

BufferRef BufferRef::trait_clone() const noexcept {
  return BufferRef{table_, index_};
}

void BufferRef::trait_copy(const BufferRef &_other) noexcept {
  table_ = _other.table_;
  index_ = _other.index_;
}

bool BufferRef::_test_null() const noexcept {
  return table_ == nullptr;
}

buffer_t &BufferRef::trait_buffer() const noexcept {
  return table_->buffers_[index_].buffer_;
}

void BufferRef::trait_release() noexcept {
  std::lock_guard<std::mutex> lock(table_->mutex_);
  table_->buffers_[index_].is_used_ = false;
  table_->size_--;
}

// Test functions
std::size_t BufferRef::_test_index() const noexcept {
  return index_;
}

}  // namespace PawnDB
