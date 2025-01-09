#ifndef PAWNDB_TYPES_LOCK_RECORDS_H
#define PAWNDB_TYPES_LOCK_RECORDS_H

#include <array>

#include "pawndb/params.h"
#include "pawndb/traits/const_iterator.h"
#include "pawndb/traits/container.h"
#include "pawndb/traits/lock_manager.h"
#include "pawndb/traits/sized.h"
#include "pawndb/types/table_tuple_key.h"

namespace PawnDB {

class LockRecordIterator;

struct LockEntry {
  TableTupleKey key;
  LockType type;
  bool is_used;
  bool is_deleted;
};

class LockRecords
    : public LockManagerTrait<LockRecords, TableTupleKey, LockEntry>,
      public ConstIteratorTrait<LockRecords, LockRecordIterator>,
      public Sized<LockRecords>,
      public Container<LockRecords> {
 private:
  constexpr static std::size_t N = MAX_LOCK_PER_TRANSACTION;
  friend class LockRecordIterator;

  std::array<LockEntry, N> locks_;
  std::size_t size_;
  TableTupleKey next_;

 public:
  LockRecords() noexcept : locks_(), size_(0), next_() {}

  LockError trait_lock(const TableTupleKey& _key, LockType _type) noexcept;
  LockError trait_unlock(const TableTupleKey& _key) noexcept;
  LockError trait_promote(const TableTupleKey& _key) noexcept;
  LockR trait_get_lock(const TableTupleKey& _key) const noexcept;

  std::size_t trait_size() const noexcept { return size_; }

  std::size_t constexpr trait_capacity() const noexcept { return N; }
  bool trait_empty() const noexcept { return size_ == 0; }
  bool trait_full() const noexcept { return size_ >= N; }

  LockRecordIterator trait_begin() const noexcept;
  LockRecordIterator trait_end() const noexcept;
};

class LockRecordIterator
    : public ConstIteratorTypeTrait<LockRecordIterator, LockEntry> {
 public:
  LockRecordIterator(const LockRecords* table, const std::size_t idx) noexcept
      : table_(table), idx_(idx) {}

  LockRecordIterator& trait_next() noexcept;
  bool trait_equals(const LockRecordIterator& other) const noexcept;
  const LockEntry& trait_deref() noexcept;

  void advance_to_valid() noexcept;

 private:
  const LockRecords* table_;
  std::size_t idx_;
};

}  // namespace PawnDB

#endif  // PAWNDB_TYPES_LOCK_RECORDS_H
