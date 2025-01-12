#ifndef PAWNDB_WORKER_TABLE_H
#define PAWNDB_WORKER_TABLE_H

#include <sys/types.h>

#include <array>
#include <cstddef>

#include "pawndb/params.h"
#include "pawndb/traits/container.h"
#include "pawndb/traits/sized.h"
#include "pawndb/traits/hash_table.h"
#include "pawndb/types/worker_thread.h"

namespace PawnDB {

struct WorkerEntry {
  using key_t = txn_id_t;

  WorkerThread worker;
  std::size_t index;
  key_t key;
  bool is_used;
  bool is_deleted;
};

class WorkerTable : public HashTableTrait<WorkerTable, WorkerEntry>,
                    public SizedTrait<WorkerTable>,
                    public ContainerTrait<WorkerTable> {
 private:
  using key_type = WorkerEntry::key_type;
  using entry_type = WorkerEntry;

  std::array<WorkerEntry, MAX_TRANSACTIONS> table_;
  std::size_t size_;

 public:
  TableR trait_insert(const entry_type& _entry) noexcept;
  TableR trait_search(const key_type& _key) noexcept;
  TableError trait_remove(const key_type& _key) noexcept;
  TableR trait_write(const entry_type& _entry) noexcept;

  std::size_t trait_size() const noexcept { return size_; }
  std::size_t constexpr trait_capacity() const noexcept {
    return MAX_TRANSACTIONS;
  }
  bool trait_empty() const noexcept { return size_ == 0; }
  bool trait_full() const noexcept { return size_ >= MAX_TRANSACTIONS; }

 public:
};

}  // namespace PawnDB

#endif
