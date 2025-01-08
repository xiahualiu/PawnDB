#ifndef PAWNDB_WORKER_TABLE_H
#define PAWNDB_WORKER_TABLE_H

#include <sys/types.h>

#include <array>
#include <atomic>
#include <cstddef>
#include <thread>

#include "pawndb/params.h"
#include "pawndb/traits/container.h"
#include "pawndb/traits/sized.h"
#include "pawndb/traits/table.h"
#include "pawndb/types/job_channel.h"
#include "pawndb/types/worker_thread.h"

namespace PawnDB {

struct WorkerEntry {
  using key_type = txn_id_t;

  WorkerThread worker;
  std::size_t index;
  key_type key;
  bool is_used;
  bool is_deleted;
};

class WorkerTable : public TableTrait<WorkerTable, WorkerEntry>,
                    public Sized<WorkerTable>,
                    public Container<WorkerTable> {
 private:
  using key_type = WorkerEntry::key_type;
  using entry_type = WorkerEntry;

  std::array<WorkerEntry, MAX_TRANSACTIONS> table_;
  std::size_t size_;

 public:
  TableR trait_insert(const entry_type& _entry) noexcept {
    auto idx = _entry.key % MAX_TRANSACTIONS;
    auto start = idx;
    do {
      if (!table_[idx].is_used || table_[idx].is_deleted) {
        table_[idx].key = _entry.key;
        table_[idx].index = idx;
        table_[idx].is_used = true;
        table_[idx].is_deleted = false;
        size_++;
        return table_[idx];
      }
      idx = (idx + 1) % MAX_TRANSACTIONS;
    } while (idx != start);
    return TableError::Full;
  }

  TableR trait_search(const key_type& _key) noexcept {
    auto idx = _key % MAX_TRANSACTIONS;
    auto start = idx;
    do {
      if (!table_[idx].is_used) {
        return TableError::NotFound;
      }
      if (table_[idx].key == _key && !table_[idx].is_deleted) {
        return table_[idx];
      }
      idx = (idx + 1) % MAX_TRANSACTIONS;
    } while (idx != start);
    return TableError::NotFound;
  }

  void trait_remove(const key_type& _key) noexcept {
    auto idx = _key % MAX_TRANSACTIONS;
    auto start = idx;
    do {
      if (!table_[idx].is_used) {
        return;
      }
      if (table_[idx].key == _key && !table_[idx].is_deleted) {
        table_[idx].is_deleted = true;
        size_--;
        return;
      }
      idx = (idx + 1) % MAX_TRANSACTIONS;
    } while (idx != start);
  }

  void trait_write(const entry_type& _entry) noexcept {
    table_[_entry.index] = _entry;
  }

 public:
};

}  // namespace PawnDB

#endif
