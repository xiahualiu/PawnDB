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
#include "pawndb/traits/sized.h"
#include "pawndb/traits/table.h"
#include "pawndb/traits/tuple_table.h"
#include "pawndb/types/student_tuple.h"

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
  // TableTrait
  TableR trait_insert(const entry_type& _entry) noexcept;
  TableR trait_search(const key_type& _key) noexcept;
  TableError trait_remove(const key_type& _key) noexcept;
  TableR trait_write(const entry_type& _entry) noexcept;
  // TupleTableTrait
  FetchR trait_wait_shared() noexcept;
  FetchR trait_wait_exclusive() noexcept;
  TupleTableError trait_promote(const key_type& _key) noexcept;
  void trait_yield(const key_type& _key) noexcept;
  void trait_release(const key_type& _key) noexcept;
  void trait_notify_s() noexcept { s_available_.notify_all(); }
  void trait_notify_x() noexcept { x_available_.notify_one(); }
  void trait_notify_not_empty() noexcept { not_empty_.notify_all(); }
  void trait_notify_not_full() noexcept { not_empty_.notify_one(); }
  // Container
  bool trait_full() const noexcept { return size_ == Rows; }
  bool trait_empty() const noexcept { return size_ == 0; }
  bool trait_capacity() const noexcept { return Rows; }
  // Sized
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
