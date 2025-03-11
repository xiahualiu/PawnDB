#include "pawndb/types/lock_table.h"

#include "pawndb/traits/lock_table.h"
#include "pawndb/traits/table.h"

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

LockType LockEntry::trait_lock_type() const noexcept {
  return type_;
}

TableTupleKey LockEntry::trait_key() const noexcept {
  return key_;
}

LockTable::table_r LockTable::trait_insert(
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

LockTable::table_r LockTable::trait_search(
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

TableError LockTable::trait_remove(const TableTupleKey& _key) noexcept {
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

LockError LockTable::trait_add_lock(const TableTupleKey& _key,
                                      LockType _type) noexcept {
  if (size_ >= N) return LockError::Full;
  auto insert_r = trait_insert(LockEntry(_key, _type));
  switch (insert_r.getError()) {
    case TableError::Conflict: return LockError::Conflict;
    default: return LockError::None;
  }
}

LockError LockTable::trait_rm_lock(const TableTupleKey& _key) noexcept {
  trait_remove(_key);
  return LockError::None;
}

LockError LockTable::trait_promote_lock(const TableTupleKey& _key) noexcept {
  trait_search(_key).unwrap().type_ = LockType::EXCLUSIVE;
  return LockError::None;
}

LockTable::lock_r LockTable::trait_get_lock(
    const TableTupleKey& _key) noexcept {
  auto search_r = trait_search(_key);
  switch (search_r.getError()) {
    case TableError::NotFound: return LockError::NotFound;
    default: return search_r.unwrap().type_;
  }
}

bool LockTable::trait_empty() const noexcept {
  return size_ == 0;
}

bool LockTable::trait_full() const noexcept {
  return size_ >= N;
}

std::size_t LockTable::trait_size() const noexcept {
  return size_;
}

LockTableIterator LockTable::trait_begin() const noexcept {
  LockTableIterator it(this, 0);
  it.advance_to_valid();
  return it;
}

LockTableIterator LockTable::trait_end() const noexcept {
  return LockTableIterator(this, N);
}

LockTableIterator::LockTableIterator(const LockTable* table,
                                       const std::size_t idx) noexcept
    : table_(table), idx_(idx) {}

LockTableIterator::LockTableIterator(
    const LockTableIterator& other) noexcept {
  table_ = other.table_;
  idx_ = other.idx_;
}

LockTableIterator& LockTableIterator::operator=(
    const LockTableIterator& other) noexcept {
  table_ = other.table_;
  idx_ = other.idx_;
  return *this;
}

LockTableIterator& LockTableIterator::trait_next() noexcept {
  idx_++;
  advance_to_valid();
  return *this;
}

bool LockTableIterator::trait_equals(
    const LockTableIterator& other) const noexcept {
  return idx_ == other.idx_;
}

const LockEntry& LockTableIterator::trait_deref() noexcept {
  return table_->locks_[idx_];
}

void LockTableIterator::advance_to_valid() noexcept {
  while (idx_ < LockTable::N &&
         (!table_->locks_[idx_].is_used_ || table_->locks_[idx_].is_deleted_)) {
    idx_++;
  }
}

}  // namespace PawnDB
