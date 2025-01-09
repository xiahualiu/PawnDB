#include "pawndb/types/student_table.h"
#include "pawndb/traits/table.h"

namespace PawnDB {

StudentTable::TableR StudentTable::trait_insert(
    const entry_type& _entry) noexcept {
  auto lock = std::unique_lock<std::mutex>(mutex_);
  not_full_.wait(lock, [&]() { return !trait_full(); });
  auto idx = tuple_key_ % Rows;
  auto start = idx;
  do {
    if (!table_[idx].is_used || table_[idx].is_deleted) {
      table_[idx] = _entry;
      table_[idx].key = tuple_key_;
      table_[idx].lock = 0;
      table_[idx].is_used = true;
      table_[idx].is_deleted = false;
      tuple_key_++;
      size_++;
      return table_[idx];
    }
    idx = (idx + 1) % Rows;
  } while (idx != start);
  return TableError::Full;
}

StudentTable::TableR StudentTable::trait_search(const key_type& _key) noexcept {
  auto lock = std::unique_lock<std::mutex>(mutex_);
  auto idx = _key % Rows;
  auto start = idx;
  do {
    if (!table_[idx].is_used) {
      return TableError::NotFound;
    }
    if (table_[idx].key == _key && !table_[idx].is_deleted) {
      return table_[idx];
    }
    idx = (idx + 1) % Rows;
  } while (idx != start);
  return TableError::NotFound;
}

TableError StudentTable::trait_remove(const key_type& _key) noexcept {
  auto lock = std::unique_lock<std::mutex>(mutex_);
  auto idx = _key % Rows;
  auto start = idx;
  do {
    if (!table_[idx].is_used) {
      return TableError::NotFound;
    }
    if (table_[idx].key == _key && !table_[idx].is_deleted) {
      table_[idx].is_deleted = true;
      size_--;
      return TableError::None;
    }
    idx = (idx + 1) % Rows;
  } while (idx != start);
  return TableError::NotFound;
}

StudentTable::TableR StudentTable::trait_write(const entry_type& _entry) noexcept {
  auto lock = std::unique_lock<std::mutex>(mutex_);
  auto idx = _entry.key % Rows;
  auto start = idx;
  do {
    if (!table_[idx].is_used) {
      return TableError::NotFound;
    }
    if (table_[idx].key == _entry.key && !table_[idx].is_deleted) {
      table_[idx].tuple = _entry.tuple;
      return table_[idx];
    }
    idx = (idx + 1) % Rows;
  } while (idx != start);
  return TableError::NotFound;
}

StudentTable::FetchR StudentTable::trait_wait_shared() noexcept {
  auto lock = std::unique_lock<std::mutex>(mutex_);
  FetchR result = TupleTableError::Timeout;
  if (not_empty_.wait_for(lock, WAIT_TIMEOUT,
                          [&]() { return !trait_empty(); })) {
    if (s_available_.wait_for(lock, WAIT_TIMEOUT, [&]() {
          for (tbl_row_t i = 0; i < Rows; i++) {
            if (table_[i].is_used && !table_[i].is_deleted &&
                table_[i].lock >= 0) {
              table_[i].lock++;
              result = table_[i];
              return true;
            }
          }
          return false;
        })) {
      return result;
    }
  }
  return result;
}

StudentTable::FetchR StudentTable::trait_wait_exclusive() noexcept {
  auto lock = std::unique_lock<std::mutex>(mutex_);
  FetchR result = TupleTableError::Timeout;

  if (not_empty_.wait_for(lock, WAIT_TIMEOUT,
                          [&]() { return !trait_empty(); })) {
    if (x_available_.wait_for(lock, WAIT_TIMEOUT, [&]() {
          for (tbl_row_t i = 0; i < Rows; i++) {
            if (table_[i].is_used && !table_[i].is_deleted &&
                table_[i].lock == 0) {
              table_[i].lock = -1;  // Set exclusive lock
              result = table_[i];
              return true;
            }
          }
          return false;
        })) {
      return result;
    }
  }
  return result;
}

TupleTableError StudentTable::trait_promote(const key_type& _key) noexcept {
  auto lock = std::unique_lock<std::mutex>(mutex_);
  auto idx = _key % Rows;
  auto start = idx;
  do {
    if (table_[idx].key == _key && !table_[idx].is_deleted) {
      if (!x_available_.wait_for(lock, WAIT_TIMEOUT, [&]() {
            return table_[idx].lock == 1;  // Still has our shared lock
          })) {
        return TupleTableError::Timeout;
      }
      table_[idx].lock = -1;
      return TupleTableError::None;
    }
    idx = (idx + 1) % Rows;
  } while (idx != start);
  return TupleTableError::NotFound;
}

void StudentTable::trait_yield(const key_type& _key) noexcept {
  auto lock = std::unique_lock<std::mutex>(mutex_);
  auto idx = _key % Rows;
  auto start = idx;
  do {
    if (table_[idx].key == _key && !table_[idx].is_deleted) {
      table_[idx].lock--;
      return;
    }
    idx = (idx + 1) % Rows;
  } while (idx != start);
}

void StudentTable::trait_release(const key_type& _key) noexcept {
  auto lock = std::unique_lock<std::mutex>(mutex_);
  auto idx = _key % Rows;
  auto start = idx;

  do {
    if (table_[idx].key == _key && !table_[idx].is_deleted) {
      if (table_[idx].lock == -1) {
        table_[idx].lock = 0;
      } else {
        table_[idx].lock--;
      }
      return;
    }
    idx = (idx + 1) % Rows;
  } while (idx != start);
}

}  // namespace PawnDB
