#ifndef PAWNDB_TYPES_COMMIT_TABLE_H
#define PAWNDB_TYPES_COMMIT_TABLE_H

#include <array>
#include <cstddef>

#include "pawndb/params.h"
#include "pawndb/traits/const_iterator.h"
#include "pawndb/traits/container.h"
#include "pawndb/traits/parser.h"
#include "pawndb/traits/sized.h"
#include "pawndb/traits/table.h"
#include "pawndb/types/table_tuple_key.h"

namespace PawnDB {

struct CommitEntry {
  using key_type = TableTupleKey;

  TableTupleKey key;
  OpType op;
  std::size_t buffer_i;
  std::size_t buffer_size;
  bool is_used;
  bool is_deleted;
};

class CommitTableIterator;

class CommitTable : public TableTrait<CommitTable, CommitEntry>,
                    public ConstIteratorTrait<CommitTable, CommitTableIterator>,
                    public Sized<CommitTable>,
                    public Container<CommitTable> {
 private:
  static constexpr std::size_t N = MAX_COMMIT_PER_TRANSACTION;

  std::array<CommitEntry, N> entries_;
  std::size_t size_{0};

 public:
  class CommitTableIterator
      : public ConstIteratorTypeTrait<CommitTableIterator, CommitEntry> {
   private:
    const CommitTable* table_;
    std::size_t index_;

   public:
    CommitTableIterator(const CommitTable* table, std::size_t index)
        : table_(table), index_(index) {}

    CommitTableIterator& trait_next() noexcept {
      index_++;
      advance_to_valid();
      return *this;
    }

    bool trait_equals(const CommitTableIterator& other) const noexcept {
      return index_ == other.index_;
    }

    const CommitEntry& trait_deref() const noexcept {
      return table_->entries_[index_];
    }

    void advance_to_valid() noexcept {
      while (index_ < N && (!table_->entries_[index_].is_used ||
                            table_->entries_[index_].is_deleted)) {
        index_++;
      }
    }
  };

  TableR trait_insert(const CommitEntry& _entry) noexcept {
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

  TableR trait_search(const TableTupleKey& _key) const noexcept {
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

  TableError trait_remove(const TableTupleKey& _key) noexcept {
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

  TableError trait_write(const CommitEntry& _entry) noexcept {
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

  CommitTableIterator trait_begin() const noexcept {
    CommitTableIterator iter(this, 0);
    iter.advance_to_valid();
    return iter;
  }

  CommitTableIterator trait_end() const noexcept {
    return CommitTableIterator(this, N);
  }

  std::size_t trait_size() const noexcept { return size_; }
  bool trait_empty() const noexcept { return size_ == 0; }
  bool trait_full() const noexcept { return size_ >= N; }
  std::size_t trait_capacity() const noexcept { return N; }
};

}  // namespace PawnDB

#endif  // PAWNDB_TYPES_COMMIT_TABLE_H
