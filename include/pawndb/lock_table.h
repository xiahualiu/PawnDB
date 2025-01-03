/**
 * @file td_lock.h
 * @author Xiahua Liu @xiahualiu
 * @brief Thread lock table class, used for lock management on a worker thread.
 * @version 0.1
 * @date 2025-01-02
 *
 * @copyright MIT License
 *
 */

#ifndef PAWNDB_THREAD_LOCK_H
#define PAWNDB_THREAD_LOCK_H

#include <sys/types.h>

#include <array>
#include <tuple>

#include "pawndb/ds/hash.h"
#include "pawndb/params.h"

namespace PawnDB {

enum class LockError { None, LockConflict, LockNotFound, LockFull, Unknown };
enum class LockType { Shared, Exclusive };

class LockTable {
 public:
  LockTable() = default;

  using key_t = std::uint16_t;

  bool full() const noexcept { return hash_table.full(); }
  bool empty() const noexcept { return hash_table.empty(); }

  LockError lock(tp_id_t _tbl, tbl_row_t _key, LockType _lk) noexcept {
    auto hash_key = key(_tbl, _key);
    auto insert_r = hash_table.insert(hash_key);
    switch (insert_r.getError()) {
      case HashError::None:
        locks[insert_r.unwrap()] = _lk;
        return LockError::None;
      case HashError::Full: return LockError::LockFull;
      case HashError::Duplicate: return LockError::LockConflict;
      default: return LockError::Unknown;
    }
  }

  LockError unlock(tp_id_t _tbl, tbl_row_t _key) noexcept {
    auto hash_key = key(_tbl, _key);
    auto search_r = hash_table.remove(hash_key);
    switch (search_r.getError()) {
      case HashError::None: return LockError::None;
      case HashError::NotFound: return LockError::LockNotFound;
      default: return LockError::Unknown;
    }
  }

  LockError promote(tp_id_t _tbl, tbl_row_t _key) noexcept {
    auto hash_key = key(_tbl, _key);
    auto search_r = hash_table.search(hash_key);
    if (!search_r) return LockError::LockNotFound;
    locks[search_r.unwrap()] = LockType::Exclusive;
    return LockError::None;
  }

  key_t key(tp_id_t _tbl, tbl_row_t _key) noexcept {
    return static_cast<key_t>(_key << 8 | _tbl);
  }

  std::pair<tp_id_t, tbl_row_t> key(key_t _key) const noexcept {
    return std::make_pair(_key & 0xff, _key >> 8);
  }

  using LockR = Result<LockType, LockError>;

  LockR get(tp_id_t _tbl, tbl_row_t _key) noexcept {
    auto hash_key = key(_tbl, _key);
    auto search_r = hash_table.search(hash_key);
    if (!search_r) return LockError::LockNotFound;
    return locks[search_r.unwrap()];
  }

  class ConstIterator {
   public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = std::tuple<tp_id_t, tbl_row_t, LockType>;
    using difference_type = std::ptrdiff_t;
    using pointer = const value_type*;
    using reference = const value_type&;

    ConstIterator(const LockTable* _table, size_t _index)
        : table(_table), index(static_cast<tbl_row_t>(_index)) {
      advance_to_valid();
    }

    value_type operator*() const {
      auto hash_key = table->hash_table[index];
      auto [tbl, key] = table->key(hash_key);
      return std::make_tuple(tbl, key, table->locks[hash_key]);
    }

    ConstIterator& operator++() {
      ++index;
      advance_to_valid();
      return *this;
    }

    ConstIterator operator++(int) {
      ConstIterator tmp = *this;
      ++(*this);
      return tmp;
    }

    friend bool operator==(const ConstIterator& a, const ConstIterator& b) {
      return a.index == b.index;
    }

    friend bool operator!=(const ConstIterator& a, const ConstIterator& b) {
      return a.index != b.index;
    }

   private:
    void advance_to_valid() {
      while (index < MAX_LOCK_PER_TRANSACTION &&
             !table->hash_table.is_valid(index)) {
        ++index;
      }
    }

    const LockTable* table;
    tbl_row_t index;
  };

  ConstIterator begin() const { return ConstIterator(this, 0); }
  ConstIterator end() const {
    return ConstIterator(this, MAX_LOCK_PER_TRANSACTION);
  }

 private:
  Hash<key_t, MAX_LOCK_PER_TRANSACTION> hash_table;
  std::array<LockType, MAX_LOCK_PER_TRANSACTION> locks;
};

}  // namespace PawnDB

#endif  // PAWNDB_THREAD_LOCK_H
