#include "pawndb/types/lock_list.h"

namespace PawnDB {

lock_list::table_r lock_list::insert(const lock_entry& _entry) noexcept {
  auto idx = _entry.key_.hash_() % N;
  while (true) {
    if (!locks_[idx].is_used_ || locks_[idx].is_deleted_) {
      locks_[idx] = _entry;
      locks_[idx].is_used_ = true;
      locks_[idx].is_deleted_ = false;
      size_++;
      return locks_[idx];
    }
    if (locks_[idx].key_.equals_(_entry.key_)) {
      return LockError::Conflict;
    }
    idx = (idx + 1) % N;
  }
}

lock_list::table_r lock_list::search(const unique_key& _key) noexcept {
  auto idx = _key.hash_() % N;
  auto start = idx;
  do {
    if (!locks_[idx].is_used_) return LockError::NotFound;
    if (locks_[idx].key_.equals_(_key) && !locks_[idx].is_deleted_) {
      return locks_[idx];
    }
    idx = (idx + 1) % N;
  } while (idx != start);
  return LockError::NotFound;
}

LockError lock_list::remove(const unique_key& _key) noexcept {
  auto idx = _key.hash_() % N;
  while (true) {
    if (locks_[idx].key_.equals_(_key) && !locks_[idx].is_deleted_) {
      locks_[idx].is_deleted_ = true;
      size_--;
      return LockError::None;
    }
    idx = (idx + 1) % N;
  }
}

void lock_list::clear() noexcept {
  for (auto& lock : locks_) {
    lock.is_used_ = false;
  }
  size_ = 0;
}

LockError lock_list::add_lock(const unique_key& _key, LockType _type) noexcept {
  if (size_ >= N) return LockError::Full;
  auto insert_r = insert(lock_entry(_key, _type));
  switch (insert_r.getError()) {
    case LockError::Conflict: return LockError::Conflict;
    default: return LockError::None;
  }
}

LockError lock_list::rm_lock(const unique_key& _key) noexcept {
  remove(_key);
  return LockError::None;
}

LockError lock_list::promote_lock(const unique_key& _key) noexcept {
  search(_key).unwrap().type_ = LockType::EXCLUSIVE;
  return LockError::None;
}

lock_list::lock_r lock_list::get_lock(const unique_key& _key) noexcept {
  auto search_r = search(_key);
  switch (search_r.getError()) {
    case LockError::NotFound: return LockError::NotFound;
    default: return search_r.unwrap().type_;
  }
}

bool lock_list::empty() const noexcept {
  return size_ == 0;
}

bool lock_list::full() const noexcept {
  return size_ >= N;
}

std::size_t lock_list::size() const noexcept {
  return size_;
}

LockRecordIterator lock_list::begin() const noexcept {
  LockRecordIterator it(this, 0);
  it.advance_to_valid();
  return it;
}

LockRecordIterator lock_list::end() const noexcept {
  return LockRecordIterator(this, N);
}

LockRecordIterator::LockRecordIterator(const lock_list* table,
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

LockRecordIterator& LockRecordIterator::next() noexcept {
  idx_++;
  advance_to_valid();
  return *this;
}

bool LockRecordIterator::equals(
    const LockRecordIterator& other) const noexcept {
  return idx_ == other.idx_;
}

const lock_entry& LockRecordIterator::deref() noexcept {
  return table_->locks_[idx_];
}

void LockRecordIterator::copy_from(const LockRecordIterator& other) noexcept {
  table_ = other.table_;
  idx_ = other.idx_;
}

LockRecordIterator LockRecordIterator::copy() const noexcept {
  return LockRecordIterator(*this);
}

void LockRecordIterator::advance_to_valid() noexcept {
  while (idx_ < lock_list::N &&
         (!table_->locks_[idx_].is_used_ || table_->locks_[idx_].is_deleted_)) {
    idx_++;
  }
}

}  // namespace PawnDB