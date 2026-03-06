#include "pawndb/types/buffer_table.h"

#include <utility>

namespace PawnDB {

BufferTable::request_r BufferTable::trait_request() noexcept {
  std::size_t index = 0;
  {
    std::lock_guard<std::mutex> lock(mutex_);
    if (size_ >= N) {
      return BufferError::Full;
    }
    while (buffers_[next_].ref_count_ > 0) {
      next_ = (next_ + 1) % N;
    }
    index = next_;
    buffers_[index].ref_count_ = 1;
    next_ = (index + 1) % N;
    size_++;
  }
  return BufferRef{this, index};
}

void BufferTable::trait_clear() noexcept {
  std::lock_guard<std::mutex> lock(mutex_);
  for (auto &buffer : buffers_) {
    buffer.ref_count_ = 0;
  }
  size_ = 0;
  next_ = 0;
}

bool BufferTable::trait_empty() const noexcept {
  std::lock_guard<std::mutex> lock(mutex_);
  return size_ == 0;
}

bool BufferTable::trait_full() const noexcept {
  std::lock_guard<std::mutex> lock(mutex_);
  return size_ >= N;
}

std::size_t BufferTable::trait_size() const noexcept {
  std::lock_guard<std::mutex> lock(mutex_);
  return size_;
}

std::uint8_t BufferTable::_test_is_used(std::size_t _i) const noexcept {
  std::lock_guard<std::mutex> lock(mutex_);
  return buffers_[_i].ref_count_ > 0;
}

std::uint16_t BufferTable::_test_ref_count(std::size_t _i) const noexcept {
  std::lock_guard<std::mutex> lock(mutex_);
  return buffers_[_i].ref_count_;
}

void BufferTable::retain_ref_(std::size_t index) noexcept {
  std::lock_guard<std::mutex> lock(mutex_);
  auto &entry = buffers_[index];
  if (entry.ref_count_ == 0) {
    size_++;
  }
  entry.ref_count_++;
}

void BufferTable::release_ref_(std::size_t index) noexcept {
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

BufferRef::BufferRef(BufferTable *_table, std::size_t _index) noexcept
    : table_(_table), index_(_index) {}

BufferRef::BufferRef(const BufferRef &_other) noexcept
    : table_(_other.table_), index_(_other.index_) {
  if (table_ != nullptr) {
    table_->retain_ref_(index_);
  }
}

BufferRef::BufferRef(BufferRef &&_other) noexcept
    : table_(_other.table_), index_(_other.index_) {
  _other.table_ = nullptr;
  _other.index_ = 0;
}

BufferRef &BufferRef::operator=(const BufferRef &_other) noexcept {
  if (this == &_other) {
    return *this;
  }
  if (_other.table_ != nullptr) {
    _other.table_->retain_ref_(_other.index_);
  }
  if (table_ != nullptr) {
    table_->release_ref_(index_);
  }
  table_ = _other.table_;
  index_ = _other.index_;
  return *this;
}

BufferRef &BufferRef::operator=(BufferRef &&_other) noexcept {
  if (this == &_other) {
    return *this;
  }
  if (table_ != nullptr) {
    table_->release_ref_(index_);
  }
  table_ = _other.table_;
  index_ = _other.index_;
  _other.table_ = nullptr;
  _other.index_ = 0;
  return *this;
}

BufferRef::~BufferRef() noexcept {
  release_ref_();
}

BufferRef BufferRef::trait_copy() const noexcept {
  return BufferRef{*this};
}

void BufferRef::trait_copy_from(const BufferRef &_other) noexcept {
  *this = _other;
}

// MoveTrait implementation
BufferRef BufferRef::trait_move() noexcept {
  // moving out of *this using the move constructor
  return std::move(*this);
}

void BufferRef::trait_move_from(BufferRef &&_other) noexcept {
  *this = std::move(_other);
}

bool BufferRef::_test_null() const noexcept {
  return table_ == nullptr;
}

buffer_t &BufferRef::trait_buffer() const noexcept {
  return table_->buffers_[index_].buffer_;
}

void BufferRef::release_ref_() noexcept {
  if (table_ == nullptr) {
    return;
  }
  auto *table = table_;
  auto index = index_;
  table_ = nullptr;
  index_ = 0;
  table->release_ref_(index);
}

// Test functions
std::size_t BufferRef::_test_index() const noexcept {
  return index_;
}

}  // namespace PawnDB
