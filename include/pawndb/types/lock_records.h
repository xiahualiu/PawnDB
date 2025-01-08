#ifndef PAWNDB_TYPES_LOCK_RECORDS_H
#define PAWNDB_TYPES_LOCK_RECORDS_H

#include <array>

#include "pawndb/params.h"
#include "pawndb/traits/container.h"
#include "pawndb/traits/lock_manager.h"
#include "pawndb/traits/sized.h"
#include "pawndb/types/lock_record_key.h"

namespace PawnDB {

class LockRecords : public LockManagerTrait<LockRecords, LockRecordKey>,
                    public Sized<LockRecords>,
                    public Container<LockRecords> {
 private:
  constexpr static std::size_t N = MAX_LOCK_PER_TRANSACTION;

  struct LockEntry {
    LockRecordKey key;
    LockType type;
    bool is_used;
    bool is_deleted;
  };

  std::array<LockEntry, N> locks_;
  std::size_t size_;
  LockRecordKey next_;

 public:
  LockRecords() noexcept : locks_(), size_(0), next_() {}

  LockError trait_lock(const LockRecordKey& _key, LockType _type) noexcept {
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

  LockError trait_unlock(const LockRecordKey& _key) noexcept {
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

  LockError trait_promote(const LockRecordKey& _key) noexcept {
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

  LockR trait_get_lock(const LockRecordKey& _key) const noexcept {
    auto idx = _key.hash() % N;
    auto start = idx;
    do {
      if (!locks_[idx].is_used) return LockError::NotFound;
      if (locks_[idx].key == _key && !locks_[idx].is_deleted) {
        return locks_[idx].type;
      }
      idx = (idx + 1) % N;
    } while (idx != start);
    return LockError::NotFound;
  }

  std::size_t trait_size() const noexcept { return size_; }
  std::size_t constexpr trait_capacity() const noexcept { return N; }
  bool trait_empty() const noexcept { return size_ == 0; }
  bool trait_full() const noexcept { return size_ >= N; }

  class Iterator {
   public:
    Iterator(const LockRecords* table, const std::size_t idx) noexcept
        : table_(table), idx_(idx) {}

    Iterator& trait_next() noexcept {
      idx_++;
      advance_to_valid();
      return *this;
    }

    bool trait_equals(const Iterator& other) const noexcept {
      return idx_ == other.idx_;
    }

    std::pair<LockRecordKey, LockType> operator*() const noexcept {
      return std::make_pair(table_->locks_[idx_].key,
                            table_->locks_[idx_].type);
    }

    void advance_to_valid() noexcept {
      while (idx_ < N && (!table_->locks_[idx_].is_used ||
                          table_->locks_[idx_].is_deleted)) {
        idx_++;
      }
    }

   private:
    const LockRecords* table_;
    std::size_t idx_;
  };

  Iterator trait_begin() const noexcept {
    auto i = Iterator(this, 0);
    i.advance_to_valid();
    return i;
  }

  Iterator trait_end() const noexcept { return Iterator(this, N); }
};

}  // namespace PawnDB

#endif  // PAWNDB_TYPES_LOCK_RECORDS_H
