#include "pawndb/types/buf_ref.h"

#include "pawndb/types/buf_table.h"

namespace PawnDB {

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
