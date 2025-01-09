#include "pawndb/types/lock_records.h"

namespace PawnDB {

LockError LockRecords::trait_lock(const TableTupleKey& _key,
                                  LockType _type) noexcept {
  auto idx = _key.hash() % N;
  auto start = idx;
  do {
    if (!locks_[idx].is_used || locks_[idx].is_deleted) {
      locks_[idx].key = _key;
      locks_[idx].type = _type;
      locks_[idx].is_used = true;
      locks_[idx].is_deleted = false;
      size_++;
      return LockError::None;
    }
    if (locks_[idx].key == _key) {
      return LockError::LockConflict;
    }
    idx = (idx + 1) % N;
  } while (idx != start);
  return LockError::Full;
}

LockError LockRecords::trait_unlock(const TableTupleKey& _key) noexcept {
  auto idx = _key.hash() % N;
  auto start = idx;
  do {
    if (!locks_[idx].is_used) return LockError::NotFound;
    if (locks_[idx].key == _key && !locks_[idx].is_deleted) {
      locks_[idx].is_deleted = true;
      size_--;
      return LockError::None;
    }
    idx = (idx + 1) % N;
  } while (idx != start);
  return LockError::NotFound;
}

LockError LockRecords::trait_promote(const TableTupleKey& _key) noexcept {
  auto idx = _key.hash() % N;
  auto start = idx;
  do {
    if (!locks_[idx].is_used) return LockError::NotFound;
    if (locks_[idx].key == _key && !locks_[idx].is_deleted) {
      if (locks_[idx].type != LockType::Shared) {
        return LockError::LockConflict;
      }
      locks_[idx].type = LockType::Exclusive;
      return LockError::None;
    }
    idx = (idx + 1) % N;
  } while (idx != start);
  return LockError::NotFound;
}

LockRecords::LockR LockRecords::trait_get_lock(
    const TableTupleKey& _key) const noexcept {
  auto idx = _key.hash() % N;
  auto start = idx;
  do {
    if (!locks_[idx].is_used) return LockError::NotFound;
    if (locks_[idx].key == _key && !locks_[idx].is_deleted) {
      return locks_[idx];
    }
    idx = (idx + 1) % N;
  } while (idx != start);
  return LockError::NotFound;
}

LockRecordIterator LockRecords::trait_begin() const noexcept {
  LockRecordIterator it(this, 0);
  it.advance_to_valid();
  return it;
}

LockRecordIterator LockRecords::trait_end() const noexcept {
  return LockRecordIterator(this, N);
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

void LockRecordIterator::advance_to_valid() noexcept {
  while (idx_ < LockRecords::N &&
         (!table_->locks_[idx_].is_used || table_->locks_[idx_].is_deleted)) {
    idx_++;
  }
}

}  // namespace PawnDB
