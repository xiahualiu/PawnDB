#include "pawndb/types/commit_table.h"

namespace PawnDB {

CommitTable::TableR CommitTable::trait_insert(
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

CommitTable::TableR CommitTable::trait_search(
    const TableTupleKey& _key) const noexcept {
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

CommitTableIterator CommitTable::trait_begin() const noexcept {
  auto it = CommitTableIterator(this, 0);
  it.advance_to_valid();
  return it;
}

CommitTableIterator CommitTable::trait_end() const noexcept {
  return CommitTableIterator(this, N);
}

CommitTableIterator& CommitTableIterator::trait_next() noexcept {
  index_++;
  advance_to_valid();
  return *this;
}

bool CommitTableIterator::trait_equals(
    const CommitTableIterator& other) const noexcept {
  return index_ == other.index_;
}

const CommitEntry& CommitTableIterator::trait_deref() const noexcept {
  return table_->entries_[index_];
}

void CommitTableIterator::advance_to_valid() noexcept {
  while (index_ < CommitTable::N && (!table_->entries_[index_].is_used ||
                                     table_->entries_[index_].is_deleted)) {
    index_++;
  }
}

}  // namespace PawnDB
