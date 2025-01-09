#include "pawndb/types/worker_table.h"

namespace PawnDB {

WorkerTable::TableR WorkerTable::trait_insert(
    const WorkerTable::entry_type& _entry) noexcept {
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

WorkerTable::TableR WorkerTable::trait_search(
    const WorkerTable::key_type& _key) noexcept {
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

void WorkerTable::trait_remove(const WorkerTable::key_type& _key) noexcept {
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

void WorkerTable::trait_write(const WorkerTable::entry_type& _entry) noexcept {
  table_[_entry.index] = _entry;
}

}  // namespace PawnDB
