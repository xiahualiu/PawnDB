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
#include <mutex>

#include "pawndb/ds/hash.h"
#include "pawndb/params.h"

namespace PawnDB {

enum class TableError { None, Timeout, UnknownError };

template <tbl_row_t Rows, typename... Ts>
class Table : private Hash<tbl_row_t, Rows> {
 private:
  using _base = Hash<tbl_row_t, Rows>;

 public:
  Table() noexcept : _base(), table(), locks(), tuple_id(0) {}

  using tuple_type = std::tuple<Ts...>;
  using WaitR = Result<tbl_row_t, TableError>;

  WaitR wait_s() noexcept {
    auto lock = std::unique_lock<std::mutex>(table_mutex);
    tbl_row_t row;
    if (not_empty.wait_for(lock, WAIT_TIMEOUT,
                           [this]() { return !this->empty(); })) {
      if (s_available.wait_for(lock, WAIT_TIMEOUT, [&]() {
            for (tbl_row_t i = 0; i < Rows; i++) {
              if (_base::table[i].is_used && locks[i] >= 0) {
                locks[i]++;
                row = i;
                return true;
              }
            }
            return false;
          })) {
        return row;
      }
    }
    return TableError::Timeout;
  }

  WaitR wait_x() noexcept {
    auto lock = std::unique_lock<std::mutex>(table_mutex);
    tbl_row_t row;
    if (not_empty.wait_for(lock, WAIT_TIMEOUT,
                           [this]() { return !this->empty(); })) {
      if (s_available.wait_for(lock, WAIT_TIMEOUT, [&]() {
            for (tbl_row_t i = 0; i < Rows; i++) {
              if (_base::table[i].is_used && locks[i] == 0) {
                locks[i] = -1;
                row = i;
                return true;
              }
            }
            return false;
          })) {
        return row;
      }
    }
    return TableError::Timeout;
  }

  TableError promote(const tbl_row_t _key) noexcept {
    auto lock = std::unique_lock<std::mutex>(table_mutex);
    auto i = _base::search(_key).unwrap();
    if (x_available.wait_for(lock, WAIT_TIMEOUT, [&]() {
          if (locks[i] == 1 || locks[i] == -1) {
            locks[i] = -1;
            return true;
          }
          return false;
        })) {
      return TableError::None;
    }
    return TableError::Timeout;
  }

  void release_s(const tbl_row_t _key) noexcept {
    auto lock = std::unique_lock<std::mutex>(table_mutex);
    auto i = _base::search(_key).unwrap();
    locks[i]--;
  }

  void release_x(const tbl_row_t _key) noexcept {
    auto lock = std::unique_lock<std::mutex>(table_mutex);
    auto i = _base::search(_key).unwrap();
    locks[i] = 0;
  }

  WaitR insert(const std::tuple<Ts...>& _tuple) noexcept {
    auto lock = std::unique_lock<std::mutex>(table_mutex);
    not_full.wait(lock, [this]() { return !this->full(); });
    auto i = _base::insert(tuple_id).unwrap();
    table[i] = _tuple;
    locks[i] = 0;
    tuple_id++;
    return i;
  }

  void remove(const tbl_row_t _key) noexcept {
    auto lock = std::unique_lock<std::mutex>(table_mutex);
    _base::remove(_key).unwrap();
  }

  void update(const tbl_row_t _key, std::tuple<Ts...> _tuple) noexcept {
    auto lock = std::unique_lock<std::mutex>(table_mutex);
    auto i = _base::search(_key).unwrap();
    table[i] = _tuple;
  }

  inline std::tuple<Ts...>& operator[](const tbl_row_t _index) noexcept {
    return table[_index];
  }

  inline tbl_row_t key_at(const tbl_row_t _index) noexcept {
    return _base::table[_index].key;
  }

  inline void notify_not_empty() noexcept { not_empty.notify_all(); }
  inline void notify_not_full() noexcept { not_full.notify_all(); }
  inline void notify_s_available() noexcept { s_available.notify_all(); }
  inline void notify_x_available() noexcept { x_available.notify_all(); }

 private:
  std::mutex table_mutex;

  std::condition_variable not_empty;
  std::condition_variable not_full;
  std::condition_variable s_available;
  std::condition_variable x_available;

  std::array<std::tuple<Ts...>, Rows> table;
  std::array<lk_t, Rows> locks;

  tbl_row_t tuple_id;
};
}  // namespace PawnDB

#endif  // PAWNDB_TABLE_H
