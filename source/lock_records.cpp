#include "pawndb/types/lock_records.h"

#include "pawndb/traits/lock_manager.h"

namespace PawnDB {

LockEntry::LockEntry(const TableTupleKey& _key, LockType _type) noexcept
    : key(_key), type(_type), is_used(true), is_deleted(false) {}

LockEntry::LockEntry(const LockEntry& other) noexcept {
  key = other.key;
  type = other.type;
  is_used = other.is_used;
  is_deleted = other.is_deleted;
}

LockEntry& LockEntry::operator=(const LockEntry& other) noexcept {
  key = other.key;
  type = other.type;
  is_used = other.is_used;
  is_deleted = other.is_deleted;
  return *this;
}

LockEntry LockEntry::trait_clone() const noexcept {
  return LockEntry(*this);
}

void LockEntry::trait_copy(const LockEntry& other) noexcept {
  key = other.key;
  type = other.type;
  is_used = other.is_used;
  is_deleted = other.is_deleted;
}

LockType LockEntry::trait_lock_type() const noexcept {
  return type;
}

const TableTupleKey& LockEntry::trait_key() const noexcept {
  return key;
}

LockRecords::table_r LockRecords::trait_insert(
    const LockEntry& _entry) noexcept {
  auto idx = _entry.key.hash() % N;
  auto start = idx;
  do {
    if (!locks_[idx].is_used || locks_[idx].is_deleted) {
      locks_[idx] = _entry;
      size_++;
      return table_r(locks_[idx]);
    }
    if (locks_[idx].key == _entry.key) {
      return TableError::Conflict;
    }
    idx = (idx + 1) % N;
  } while (idx != start);
  return TableError::Full;
}

LockRecords::table_r LockRecords::trait_search(
    const TableTupleKey& _key) noexcept {
  auto idx = _key.hash() % N;
  auto start = idx;
  do {
    if (!locks_[idx].is_used) return TableError::NotFound;
    if (locks_[idx].key == _key && !locks_[idx].is_deleted) {
      return locks_[idx];
    }
    idx = (idx + 1) % N;
  } while (idx != start);
  return TableError::NotFound;
}

TableError LockRecords::trait_remove(const TableTupleKey& _key) noexcept {
  auto idx = _key.hash() % N;
  auto start = idx;
  do {
    if (!locks_[idx].is_used) return TableError::NotFound;
    if (locks_[idx].key == _key && !locks_[idx].is_deleted) {
      locks_[idx].is_deleted = true;
      size_--;
      return TableError::None;
    }
    idx = (idx + 1) % N;
  } while (idx != start);
  return TableError::NotFound;
}

TableError LockRecords::trait_write(const LockEntry& _entry) noexcept {
  auto idx = _entry.key.hash() % N;
  auto start = idx;
  do {
    if (!locks_[idx].is_used) return TableError::NotFound;
    if (locks_[idx].key == _entry.key && !locks_[idx].is_deleted) {
      locks_[idx] = _entry;
      return TableError::None;
    }
    idx = (idx + 1) % N;
  } while (idx != start);
  return TableError::NotFound;
}

LockError LockRecords::trait_add_lock(const TableTupleKey& _key,
                                      LockType _type) noexcept {
  if (size_ >= N) return LockError::Full;
  auto insert_r = trait_insert(LockEntry(_key, _type));
  switch (insert_r.getError()) {
    case TableError::Conflict: return LockError::AlreadyHeld;
    case TableError::Full: return LockError::Full;
    default: return LockError::None;
  }
}

LockError LockRecords::trait_rm_lock(const TableTupleKey& _key) noexcept {
  auto remove_r = trait_remove(_key);
  switch (remove_r) {
    case TableError::NotFound: return LockError::NotFound;
    default: return LockError::None;
  }
}

LockError LockRecords::trait_promote_lock(const TableTupleKey& _key) noexcept {
  auto search_r = trait_search(_key);
  switch (search_r.getError()) {
    case TableError::NotFound: return LockError::NotFound;
    default: {
      auto& lock = search_r.unwrap();
      if (lock.type != LockType::SHARED) return LockError::InvalidType;
      lock.type = LockType::EXCLUSIVE;
      return LockError::None;
    }
  }
}

LockRecords::LockR LockRecords::trait_get_lock(
    const TableTupleKey& _key) noexcept {
  auto search_r = trait_search(_key);
  switch (search_r.getError()) {
    case TableError::NotFound: return LockError::NotFound;
    default: return search_r.unwrap().type;
  }
}

bool LockRecords::trait_empty() const noexcept {
    return size_ == 0;
}

bool LockRecords::trait_full() const noexcept {
    return size_ >= N;
}

std::size_t LockRecords::trait_size() const noexcept {
    return size_;
}

LockRecordIterator LockRecords::trait_begin() const noexcept {
  LockRecordIterator it(this, 0);
  it.advance_to_valid();
  return it;
}

LockRecordIterator LockRecords::trait_end() const noexcept {
  return LockRecordIterator(this, N);
}

LockRecordIterator::LockRecordIterator(const LockRecords* table,
                                       const std::size_t idx) noexcept
    : table_(table), idx_(idx) {}

LockRecordIterator::LockRecordIterator(
    const LockRecordIterator& other) noexcept {
  table_ = other.table_;
  idx_ = other.idx_;
}

LockRecordIterator& LockRecordIterator::operator=(
    const LockRecordIterator& other) noexcept {
  table_ = other.table_;
  idx_ = other.idx_;
  return *this;
}

LockRecordIterator& LockRecordIterator::trait_next() noexcept {
  idx_++;
  advance_to_valid();
  return *this;
}

bool LockRecordIterator::trait_equals(
    const LockRecordIterator& other) const noexcept {
  return idx_ == other.idx_;
}

const LockEntry& LockRecordIterator::trait_deref() noexcept {
  return table_->locks_[idx_];
}

void LockRecordIterator::trait_copy(const LockRecordIterator& other) noexcept {
  table_ = other.table_;
  idx_ = other.idx_;
}

LockRecordIterator LockRecordIterator::trait_clone() const noexcept {
  return LockRecordIterator(*this);
}

void LockRecordIterator::advance_to_valid() noexcept {
  while (idx_ < LockRecords::N &&
         (!table_->locks_[idx_].is_used || table_->locks_[idx_].is_deleted)) {
    idx_++;
  }
}

}  // namespace PawnDB
