#include "pawndb/types/lock_records.h"

#include "pawndb/traits/hash_table.h"
#include "pawndb/traits/lock_manager.h"

namespace PawnDB {

LockEntry::LockEntry(const TableTupleKey& _key, LockType _type) noexcept
    : key_(_key), type_(_type), is_used_(true), is_deleted_(false) {}

LockEntry::LockEntry(const LockEntry& other) noexcept {
  key_ = other.key_;
  type_ = other.type_;
}

LockEntry& LockEntry::operator=(const LockEntry& other) noexcept {
  key_ = other.key_;
  type_ = other.type_;
  return *this;
}

LockEntry LockEntry::trait_clone() const noexcept {
  return LockEntry(*this);
}

void LockEntry::trait_copy(const LockEntry& other) noexcept {
  key_ = other.key_;
  type_ = other.type_;
}

LockType LockEntry::trait_lock_type() const noexcept {
  return type_;
}

const TableTupleKey& LockEntry::trait_key() const noexcept {
  return key_;
}

LockRecords::table_r LockRecords::trait_insert(
    const LockEntry& _entry) noexcept {
  auto idx = _entry.key_.hash() % N;
  while (true) {
    if (!locks_[idx].is_used_ || locks_[idx].is_deleted_) {
      locks_[idx] = _entry;
      locks_[idx].is_used_ = true;
      locks_[idx].is_deleted_ = false;
      size_++;
      return locks_[idx];
    }
    if (locks_[idx].key_ == _entry.key_) {
      return TableError::Conflict;
    }
    idx = (idx + 1) % N;
  }
}

LockRecords::table_r LockRecords::trait_search(
    const TableTupleKey& _key) noexcept {
  auto idx = _key.hash() % N;
  auto start = idx;
  do {
    if (!locks_[idx].is_used_) return TableError::NotFound;
    if (locks_[idx].key_ == _key && !locks_[idx].is_deleted_) {
      return locks_[idx];
    }
    idx = (idx + 1) % N;
  } while (idx != start);
  return TableError::NotFound;
}

TableError LockRecords::trait_remove(const TableTupleKey& _key) noexcept {
  auto idx = _key.hash() % N;
  while (true) {
    if (locks_[idx].key_ == _key && !locks_[idx].is_deleted_) {
      locks_[idx].is_deleted_ = true;
      size_--;
      return TableError::None;
    }
    idx = (idx + 1) % N;
  }
}

// Hide definition of trait_write because it is not used
// TableError LockRecords::trait_write(const LockEntry& _entry) noexcept;

void LockRecords::trait_clear() noexcept {
  for (auto& lock : locks_) {
    lock.is_used_ = false;
  }
  size_ = 0;
}

LockError LockRecords::trait_add_lock(const TableTupleKey& _key,
                                      LockType _type) noexcept {
  if (size_ >= N) return LockError::Full;
  auto insert_r = trait_insert(LockEntry(_key, _type));
  switch (insert_r.getError()) {
    case TableError::Conflict: return LockError::AlreadyHeld;
    default: return LockError::None;
  }
}

LockError LockRecords::trait_rm_lock(const TableTupleKey& _key) noexcept {
  trait_remove(_key);
  return LockError::None;
}

LockError LockRecords::trait_promote_lock(const TableTupleKey& _key) noexcept {
  trait_search(_key).unwrap().type_ = LockType::EXCLUSIVE;
  return LockError::None;
}

LockRecords::LockR LockRecords::trait_get_lock(
    const TableTupleKey& _key) noexcept {
  auto search_r = trait_search(_key);
  switch (search_r.getError()) {
    case TableError::NotFound: return LockError::NotFound;
    default: return search_r.unwrap().type_;
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
         (!table_->locks_[idx_].is_used_ || table_->locks_[idx_].is_deleted_)) {
    idx_++;
  }
}

}  // namespace PawnDB
