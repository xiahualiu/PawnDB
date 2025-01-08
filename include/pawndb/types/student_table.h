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

#include <array>
#include <condition_variable>
#include <cstddef>
#include <mutex>
#include <utility>

#include "pawndb/params.h"
#include "pawndb/traits/container.h"
#include "pawndb/traits/tuple_table.h"
#include "pawndb/types/student_key.h"
#include "pawndb/types/student_tuple.h"

namespace PawnDB {

template <tbl_row_t Rows, typename... Ts>
struct TableStruct {};

class StudentTable
    : public TupleTableTrait<StudentTable, StudentKey, StudentTuple>,
      public Sized<StudentTable>,
      public Container<StudentTable> {
  constexpr static std::size_t Rows = 10;

 public:
  constexpr static std::size_t trait_id() noexcept { return 1; }

  FetchR trait_insert(const StudentTuple& _tuple) noexcept {
    auto lock = std::unique_lock<std::mutex>(mutex_);
    not_full_.wait(lock, [&]() { return !full(); });
    auto idx = tuple_key_.hash() % Rows;
    auto start = idx;
    do {
      if (!table_[idx].is_used || table_[idx].is_deleted) {
        table_[idx].key = tuple_key_;
        table_[idx].tuple = _tuple;
        table_[idx].lock = 0;
        table_[idx].is_used = true;
        table_[idx].is_deleted = false;
        tuple_key_.next();
        size_++;
        return FetchR{std::pair(&table_[idx].tuple, tuple_key_)};
      }
      idx = (idx + 1) % Rows;
    } while (idx != start);
    return TableError::Full;
  }

  void trait_remove(const StudentKey& _key) noexcept {
    auto lock = std::unique_lock<std::mutex>(mutex_);
    auto idx = _key.hash() % Rows;
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

  void trait_write(const StudentKey& _key,
                   const StudentTuple& _tuple) noexcept {
    auto lock = std::unique_lock<std::mutex>(mutex_);
    auto idx = _key.hash() % Rows;
    auto start = idx;
    do {
      if (!table_[idx].is_used) {
        return;
      }
      if (table_[idx].key == _key && !table_[idx].is_deleted) {
        table_[idx].tuple = _tuple;
        return;
      }
      idx = (idx + 1) % Rows;
    } while (idx != start);
  }

  FetchR trait_wait_shared() noexcept {
    auto lock = std::unique_lock<std::mutex>(mutex_);
    FetchR result = TableError::Timeout;
    if (not_empty_.wait_for(lock, WAIT_TIMEOUT, [&]() { return !empty(); })) {
      if (s_available_.wait_for(lock, WAIT_TIMEOUT, [&]() {
            for (tbl_row_t i = 0; i < Rows; i++) {
              if (table_[i].is_used && !table_[i].is_deleted &&
                  table_[i].lock >= 0) {
                table_[i].lock++;
                result = std::pair(&table_[i].tuple, table_[i].key);
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
    FetchR result = TableError::Timeout;

    if (not_empty_.wait_for(lock, WAIT_TIMEOUT, [&]() { return !empty(); })) {
      if (x_available_.wait_for(lock, WAIT_TIMEOUT, [&]() {
            for (tbl_row_t i = 0; i < Rows; i++) {
              if (table_[i].is_used && !table_[i].is_deleted &&
                  table_[i].lock == 0) {
                table_[i].lock = -1;  // Set exclusive lock
                result = std::pair(&table_[i].tuple, table_[i].key);
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

  TableError trait_promote(const StudentKey& _key) noexcept {
    auto lock = std::unique_lock<std::mutex>(mutex_);
    auto idx = _key.hash() % Rows;
    auto start = idx;
    do {
      if (table_[idx].key == _key && !table_[idx].is_deleted) {
        if (!x_available_.wait_for(lock, WAIT_TIMEOUT, [&]() {
              return table_[idx].lock == 1;  // Still has our shared lock
            })) {
          return TableError::Timeout;
        }
        table_[idx].lock = -1;
        return TableError::None;
      }
      idx = (idx + 1) % Rows;
    } while (idx != start);
    return TableError::NotFound;
  }

  void trait_yield(const StudentKey& _key) noexcept {
    auto lock = std::unique_lock<std::mutex>(mutex_);
    auto idx = _key.hash() % Rows;
    auto start = idx;
    do {
      if (table_[idx].key == _key && !table_[idx].is_deleted) {
        table_[idx].lock--;
        return;
      }
      idx = (idx + 1) % Rows;
    } while (idx != start);
  }

  void trait_release(const StudentKey& _key) noexcept {
    auto lock = std::unique_lock<std::mutex>(mutex_);
    auto idx = _key.hash() % Rows;
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
  struct Entry {
    StudentTuple tuple;
    StudentKey key;
    lk_t lock;
    bool is_used;
    bool is_deleted;
  };

  std::mutex mutex_;
  std::condition_variable not_empty_;
  std::condition_variable not_full_;
  std::condition_variable s_available_;
  std::condition_variable x_available_;

  std::array<Entry, Rows> table_;

  std::size_t size_{0};
  StudentKey tuple_key_;
};
}  // namespace PawnDB

#endif  // PAWNDB_TABLE_H
