#include "pawndb/types/buffer_table.h"

#include "pawndb/params.h"

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

buf_ref::buf_ref(buf_table *_table, std::size_t _index) noexcept
    : table_(_table), index_(_index) {}

buf_ref::buf_ref(const buf_ref &_other) noexcept
    : table_(_other.table_), index_(_other.index_) {
  if (table_ != nullptr) {
    table_->retain_ref(index_);
  }
}

buf_ref::buf_ref(buf_ref &&_other) noexcept
    : table_(_other.table_), index_(_other.index_) {
  _other.table_ = nullptr;
  _other.index_ = 0;
}

buf_ref &buf_ref::operator=(const buf_ref &_other) noexcept {
  if (this == &_other) {
    return *this;
  }
  if (_other.table_ != nullptr) {
    _other.table_->retain_ref(_other.index_);
  }
  if (table_ != nullptr) {
    table_->release_ref(index_);
  }
  table_ = _other.table_;
  index_ = _other.index_;
  return *this;
}

buf_ref &buf_ref::operator=(buf_ref &&_other) noexcept {
  if (this == &_other) {
    return *this;
  }
  if (table_ != nullptr) {
    table_->release_ref(index_);
  }
  table_ = _other.table_;
  index_ = _other.index_;
  _other.table_ = nullptr;
  _other.index_ = 0;
  return *this;
}

buf_ref::~buf_ref() noexcept {
  release_ref_();
}

buf_t &buf_ref::buffer() const noexcept {
  return table_->buffers_[index_].buffer_;
}

void buf_ref::release_ref_() noexcept {
  if (table_ == nullptr) {
    return;
  }
  auto *table = table_;
  auto index = index_;
  table_ = nullptr;
  index_ = 0;
  table->release_ref(index);
}

}  // namespace PawnDB
