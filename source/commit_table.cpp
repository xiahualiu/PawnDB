#include "pawndb/types/commit_table.h"

#include "pawndb/traits/hash_table.h"

namespace PawnDB {

CommitEntry::CommitEntry(const TableTupleKey& _key, OpType _op,
                         const BufferRef& _buffer) noexcept
    : buffer_(_buffer),
      key_(_key),
      op_(_op),
      is_used_(true),
      is_deleted_(false) {}

CommitEntry::CommitEntry(const CommitEntry& other) noexcept
    : buffer_(other.buffer_),
      key_(other.key_),
      op_(other.op_),
      is_used_(other.is_used_),
      is_deleted_(other.is_deleted_) {}

CommitEntry& CommitEntry::operator=(const CommitEntry& other) noexcept {
  buffer_ = other.buffer_;
  key_ = other.key_;
  op_ = other.op_;
  is_used_ = other.is_used_;
  is_deleted_ = other.is_deleted_;
  return *this;
}

std::size_t CommitEntry::trait_hash() const noexcept {
  return key_.hash();
}

CommitEntry CommitEntry::trait_clone() const noexcept {
  return CommitEntry(*this);
}

void CommitEntry::trait_copy(const CommitEntry& other) noexcept {
  *this = other;
}

/** @brief Get buffer reference */
BufferRef CommitEntry::trait_buffer() const noexcept {
  return buffer_;
}

/** @brief Get table-tuple key */
const TableTupleKey& CommitEntry::trait_key() const noexcept {
  return key_;
}

/** @brief Get operation type */
OpType CommitEntry::trait_op() const noexcept {
  return op_;
}

CommitIt::CommitIt(const CommitTable* table, std::size_t index) noexcept
    : table_(table), index_(index) {}

CommitIt::CommitIt(const CommitIt& other) noexcept {
  trait_copy(other);
}

CommitIt& CommitIt::operator=(const CommitIt& other) noexcept {
  trait_copy(other);
  return *this;
}

CommitIt& CommitIt::trait_next() noexcept {
  if (index_ < table_->N) {
    ++index_;
    advance_to_valid();
  }
  return *this;
}

const CommitEntry& CommitIt::trait_deref() const noexcept {
  return table_->entries_[index_];
}

bool CommitIt::trait_equals(const CommitIt& other) const noexcept {
  return table_ == other.table_ && index_ == other.index_;
}

void CommitIt::advance_to_valid() noexcept {
  while (index_ < table_->N && (!table_->entries_[index_].is_used_ ||
                                table_->entries_[index_].is_deleted_)) {
    ++index_;
  }
}

CommitError CommitTable::trait_add_commit(const CommitEntry& _entry) noexcept {
  if (trait_full()) {
    return CommitError::Full;
  }

  auto result = trait_insert(_entry);
  switch (result.getError()) {
    case TableError::Full: return CommitError::Full;
    default: return CommitError::None;
  }
}

CommitTable::table_r CommitTable::trait_insert(const CommitEntry& _entry) noexcept {
  auto idx = _entry.key_.hash() % N;
  auto start = idx;
  do {
    if (!entries_[idx].is_used_ || entries_[idx].is_deleted_) {
      entries_[idx] = _entry;
      entries_[idx].is_used_ = true;
      entries_[idx].is_deleted_ = false;
      size_++;
      return entries_[idx];
    }
    idx = (idx + 1) % N;
  } while (idx != start);
  return TableError::Full;
}

CommitTable::table_r CommitTable::trait_search(
    const TableTupleKey& _key) noexcept {
  auto idx = _key.hash() % N;
  auto start = idx;
  do {
    if (!entries_[idx].is_used_) return TableError::NotFound;
    if (entries_[idx].key_ == _key && !entries_[idx].is_deleted_) {
      return entries_[idx];
    }
    idx = (idx + 1) % N;
  } while (idx != start);
  return TableError::NotFound;
}

TableError CommitTable::trait_remove(const TableTupleKey& _key) noexcept {
  auto idx = _key.hash() % N;
  auto start = idx;
  do {
    if (!entries_[idx].is_used_) return TableError::NotFound;
    if (entries_[idx].key_ == _key && !entries_[idx].is_deleted_) {
      entries_[idx].is_deleted_ = true;
      size_--;
      return TableError::None;
    }
    idx = (idx + 1) % N;
  } while (idx != start);
  return TableError::NotFound;
}

TableError CommitTable::trait_write(const CommitEntry& _entry) noexcept {
  auto idx = _entry.key_.hash() % N;
  auto start = idx;
  do {
    if (!entries_[idx].is_used_) return TableError::NotFound;
    if (entries_[idx].key_ == _entry.key_ && !entries_[idx].is_deleted_) {
      entries_[idx] = _entry;
      return TableError::None;
    }
    idx = (idx + 1) % N;
  } while (idx != start);
  return TableError::NotFound;
}

void CommitTable::trait_clear() noexcept {
  for (auto& entry : entries_) {
    entry.is_used_ = false;
    entry.is_deleted_ = false;
  }
  size_ = 0;
}

CommitIt CommitTable::trait_begin() const noexcept {
  auto it = CommitIt(this, 0);
  it.advance_to_valid();
  return it;
}

CommitIt CommitTable::trait_end() const noexcept {
  return CommitIt(this, N);
}

std::size_t CommitTable::trait_size() const noexcept {
  return size_;
}

bool CommitTable::trait_empty() const noexcept {
  return size_ == 0;
}

bool CommitTable::trait_full() const noexcept {
  return size_ >= N;
}

}  // namespace PawnDB
