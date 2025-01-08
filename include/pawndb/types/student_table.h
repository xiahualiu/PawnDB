/**
 * @file table.h
 * @author Xiahua Liu @xiahualiu
 * @brief PawnDB table class template.
 * @version 0.1
 * @date 2025-01-02
 *
 * @copyright MIT License
 *
 */

#ifndef PAWNDB_TABLE_H
#define PAWNDB_TABLE_H

#include <sys/types.h>

#include <array>
#include <condition_variable>
#include <cstddef>
#include <mutex>

#include "pawndb/params.h"
#include "pawndb/traits/container.h"
#include "pawndb/traits/table.h"
#include "pawndb/traits/tuple_table.h"
#include "pawndb/types/student_tuple.h"
#include "pawndb/traits/sized.h"

namespace PawnDB {

struct StudentTableEntry {
  using key_type = tbl_row_t;

  StudentTuple tuple;
  tbl_row_t key;
  lk_t lock;
  bool is_used;
  bool is_deleted;
};

class StudentTable : public TableTrait<StudentTable, StudentTableEntry>,
                     public TupleTableTrait<StudentTable, StudentTableEntry>,
                     public Sized<StudentTable>,
                     public Container<StudentTable> {
  constexpr static std::size_t Rows = 10;

 public:
  using key_type = tbl_row_t;
  using entry_type = StudentTableEntry;

  constexpr static std::size_t trait_id() noexcept { return 1; }

  TableR trait_insert(const entry_type& _entry) noexcept {
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

  TableR trait_search(const key_type& _key) noexcept {
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

  void trait_remove(const key_type& _key) noexcept {
    auto lock = std::unique_lock<std::mutex>(mutex_);
    auto idx = _key % Rows;
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
      idx = (idx + 1) % Rows;
    } while (idx != start);
  }

  void trait_write(const entry_type& _entry) noexcept {
    auto lock = std::unique_lock<std::mutex>(mutex_);
    auto idx = _entry.key % Rows;
    auto start = idx;
    do {
      if (!table_[idx].is_used) {
        return;
      }
      if (table_[idx].key == _entry.key && !table_[idx].is_deleted) {
        table_[idx] = _entry;
        return;
      }
      idx = (idx + 1) % Rows;
    } while (idx != start);
  }

  FetchR trait_wait_shared() noexcept {
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

  FetchR trait_wait_exclusive() noexcept {
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

  TupleTableError trait_promote(const key_type& _key) noexcept {
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

  void trait_yield(const key_type& _key) noexcept {
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

  void trait_release(const key_type& _key) noexcept {
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

  void trait_notify_s() noexcept { s_available_.notify_all(); }
  void trait_notify_x() noexcept { x_available_.notify_one(); }
  void trait_notify_not_empty() noexcept { not_empty_.notify_all(); }
  void trait_notify_not_full() noexcept { not_empty_.notify_one(); }

  bool trait_full() const noexcept { return size_ == Rows; }
  bool trait_empty() const noexcept { return size_ == 0; }
  bool trait_capacity() const noexcept { return Rows; }
  std::size_t trait_size() const noexcept { return size_; }

 private:
  std::mutex mutex_;
  std::condition_variable not_empty_;
  std::condition_variable not_full_;
  std::condition_variable s_available_;
  std::condition_variable x_available_;

  std::array<StudentTableEntry, Rows> table_;

  std::size_t size_;
  key_type tuple_key_;
};
}  // namespace PawnDB

#endif  // PAWNDB_TABLE_H
