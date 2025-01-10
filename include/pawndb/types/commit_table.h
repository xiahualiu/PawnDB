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
#include "pawndb/types/buffer_table.h"
#include "pawndb/types/table_tuple_key.h"

namespace PawnDB {

struct CommitEntry {
  using key_type = TableTupleKey;

  TableTupleKey key;
  OpType op;
  BufferRC buffer;
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

  friend class CommitTableIterator;

 public:
  // TableTrait
  TableR trait_insert(const CommitEntry& _entry) noexcept;
  TableR trait_search(const TableTupleKey& _key) noexcept;
  TableError trait_remove(const TableTupleKey& _key) noexcept;
  TableError trait_write(const CommitEntry& _entry) noexcept;
  // Iterator
  CommitTableIterator trait_begin() const noexcept;
  CommitTableIterator trait_end() const noexcept;
  // Sized
  std::size_t trait_size() const noexcept { return size_; }
  // Container
  bool trait_empty() const noexcept { return size_ == 0; }
  bool trait_full() const noexcept { return size_ >= N; }
  std::size_t trait_capacity() const noexcept { return N; }
};

class CommitTableIterator
    : public ConstIteratorTypeTrait<CommitTableIterator, CommitEntry> {
 private:
  const CommitTable* table_;
  std::size_t index_;

 public:
  CommitTableIterator(const CommitTable* table, std::size_t index)
      : table_(table), index_(index) {}

  CommitTableIterator& trait_next() noexcept;
  bool trait_equals(const CommitTableIterator& other) const noexcept;
  const CommitEntry& trait_deref() const noexcept;
  void advance_to_valid() noexcept;
};

}  // namespace PawnDB

#endif  // PAWNDB_TYPES_COMMIT_TABLE_H
