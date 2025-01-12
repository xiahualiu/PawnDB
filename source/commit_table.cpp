#include "pawndb/types/commit_table.h"

namespace PawnDB {

CommitEntry::CommitEntry(const TableTupleKey& _key, OpType _op,
                         const BufferRef& _buffer) noexcept
    : buffer(_buffer), key(_key), op(_op), is_used(true), is_deleted(false) {}

CommitEntry::CommitEntry(const CommitEntry& other) noexcept
    : buffer(other.buffer),
      key(other.key),
      op(other.op),
      is_used(other.is_used),
      is_deleted(other.is_deleted) {}

CommitEntry& CommitEntry::operator=(const CommitEntry& other) noexcept {
  buffer = other.buffer;
  key = other.key;
  op = other.op;
  is_used = other.is_used;
  is_deleted = other.is_deleted;
  return *this;
}

std::size_t CommitEntry::trait_hash() const noexcept {
  return key.hash();
}

CommitEntry CommitEntry::trait_clone() const noexcept {
  return CommitEntry(*this);
}

void CommitEntry::trait_copy(const CommitEntry& other) noexcept {
  *this = other;
}

/** @brief Get buffer reference */
BufferRef& CommitEntry::trait_buffer() noexcept {
  return buffer;
}

/** @brief Get table-tuple key */
const TableTupleKey& CommitEntry::trait_key() const noexcept {
  return key;
}

/** @brief Get operation type */
OpType CommitEntry::trait_op() const noexcept {
  return op;
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
  while (index_ < table_->N && (!table_->entries_[index_].is_used ||
                                table_->entries_[index_].is_deleted)) {
    ++index_;
  }
}

CommitError CommitTable::trait_add_commit(const CommitEntry& _entry) noexcept {
  if (trait_full()) {
    return CommitError::Full;
  }

  auto result = trait_insert(_entry);
  if (!result) {
    switch (result.getError()) {
      case TableError::Full: return CommitError::Full;
      case TableError::Conflict: return CommitError::AlreadyExists;
      default: return CommitError::Unknown;
    }
  }

  return CommitError::None;
}

CommitTable::table_r CommitTable::trait_insert(
    const CommitEntry& _entry) noexcept {
  if (size_ >= N) return TableError::Full;
  auto idx = _entry.key.hash() % N;
  auto start = idx;
  do {
    if (!entries_[idx].is_used || entries_[idx].is_deleted) {
      entries_[idx] = _entry;
      entries_[idx].is_used = true;
      entries_[idx].is_deleted = false;
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
    if (!entries_[idx].is_used) return TableError::NotFound;
    if (entries_[idx].key == _key && !entries_[idx].is_deleted) {
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
    if (!entries_[idx].is_used) return TableError::NotFound;
    if (entries_[idx].key == _key && !entries_[idx].is_deleted) {
      entries_[idx].is_deleted = true;
      size_--;
      return TableError::None;
    }
    idx = (idx + 1) % N;
  } while (idx != start);
  return TableError::NotFound;
}

TableError CommitTable::trait_write(const CommitEntry& _entry) noexcept {
  auto idx = _entry.key.hash() % N;
  auto start = idx;
  do {
    if (!entries_[idx].is_used) return TableError::NotFound;
    if (entries_[idx].key == _entry.key && !entries_[idx].is_deleted) {
      entries_[idx] = _entry;
      entries_[idx].is_used = true;
      entries_[idx].is_deleted = false;
      return TableError::None;
    }
    idx = (idx + 1) % N;
  } while (idx != start);
  return TableError::NotFound;
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
