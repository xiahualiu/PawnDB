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

/**
 * @brief Table data structure.
 *
 * @tparam Rows The number of rows in the table.
 * @tparam Ts The types of the columns in the table.
 */
template <tbl_row_t Rows, typename... Ts>
class Table : private Hash<tbl_row_t, Rows> {
 private:
  using _base = Hash<tbl_row_t, Rows>;

 public:
  Table() noexcept : _base(), table(), locks(), tuple_id(0) {}

  using tuple_type = std::tuple<Ts...>;
  using WaitR = Result<tbl_row_t, TableError>;

  /**
   * @brief Waits for a row to become available and locks it for shared access.
   *
   * This function waits for a row in the table to become available within a
   * specified timeout period. It first waits for the table to be non-empty, and
   * then it waits for a row to be available and locks it. If a row is
   * successfully locked, it returns the row index. If the operation times out,
   * it returns TableError::Timeout.
   *
   * @return WaitR The index of the locked row, or TableError::Timeout if the
   * operation times out.
   */
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

  /**
   * @brief Waits for a row to become available and locks it with exclusive
   * access.
   *
   * This function waits for a row in the table to become available within a
   * specified timeout period. It first waits for the table to be non-empty, and
   * then it waits for a row to be available and locks it. If a row is
   * successfully locked, it returns the row index. If the operation times out,
   * it returns TableError::Timeout.
   *
   * @return WaitR The index of the locked row, or TableError::Timeout if the
   * operation times out.
   */
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

  /**
   * @brief Promotes a row in the table to a locked state.
   *
   * This function attempts to promote a row identified by the given key to a
   * locked state. It acquires a unique lock on the table mutex and searches for
   * the row using the provided key. If the row is found and its lock state is
   * either 1 (unlocked) or -1 (already locked), it sets the lock state to -1
   * (locked). The function waits for a specified timeout duration for the lock
   * state to change.
   *
   * @param _key The key identifying the row to be promoted.
   * @return TableError::None if the row is successfully promoted to a locked
   * state.
   * @return TableError::Timeout if the operation times out before the row can
   * be promoted.
   */
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

  /**
   * @brief Releases a shared lock on a tuple.
   *
   * @param _key The key of the tuple to release.
   */
  void release_s(const tbl_row_t _key) noexcept {
    auto lock = std::unique_lock<std::mutex>(table_mutex);
    auto i = _base::search(_key).unwrap();
    locks[i]--;
  }

  /**
   * @brief Releases an exclusive lock on a tuple.
   *
   * @param _key The key of the tuple to release.
   */
  void release_x(const tbl_row_t _key) noexcept {
    auto lock = std::unique_lock<std::mutex>(table_mutex);
    auto i = _base::search(_key).unwrap();
    locks[i] = 0;
  }

  /**
   * @brief Inserts a tuple into the table.
   *
   * @param _tuple The tuple to insert.
   * @return WaitR The index of the inserted tuple, or an error if the table is
   * full.
   */
  WaitR insert(const std::tuple<Ts...>& _tuple) noexcept {
    auto lock = std::unique_lock<std::mutex>(table_mutex);
    not_full.wait(lock, [this]() { return !this->full(); });
    auto i = _base::insert(tuple_id).unwrap();
    table[i] = _tuple;
    locks[i] = 0;
    tuple_id++;
    return i;
  }

  /**
   * @brief Removes a tuple from the table.
   *
   * @param _key The key of the tuple to remove.
   */
  void remove(const tbl_row_t _key) noexcept {
    auto lock = std::unique_lock<std::mutex>(table_mutex);
    _base::remove(_key).unwrap();
  }

  /**
   * @brief Updates a tuple in the table.
   *
   * @param _key The key of the tuple to update.
   * @param _tuple The new tuple value.
   */
  void update(const tbl_row_t _key, std::tuple<Ts...> _tuple) noexcept {
    auto lock = std::unique_lock<std::mutex>(table_mutex);
    auto i = _base::search(_key).unwrap();
    table[i] = _tuple;
  }

  /**
   * @brief Accesses a tuple by index.
   *
   * @param _index The index of the tuple to access.
   * @return A reference to the tuple at the specified index.
   */
  inline std::tuple<Ts...>& operator[](const tbl_row_t _index) noexcept {
    return table[_index];
  }

  /**
   * @brief Gets the key at the specified index.
   *
   * @param _index The index of the key to get.
   * @return The key at the specified index.
   */
  inline tbl_row_t key_at(const tbl_row_t _index) noexcept {
    return _base::table[_index].key;
  }

  /**
   * @brief Notifies all waiting threads that the table is not empty.
   */
  inline void notify_not_empty() noexcept { not_empty.notify_all(); }

  /**
   * @brief Notifies all waiting threads that the table is not full.
   */
  inline void notify_not_full() noexcept { not_full.notify_all(); }

  /**
   * @brief Notifies all waiting threads that a shared lock is available.
   */
  inline void notify_s_available() noexcept { s_available.notify_all(); }

  /**
   * @brief Notifies all waiting threads that an exclusive lock is available.
   */
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
