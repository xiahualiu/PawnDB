/**
 * @file ring.h
 * @author Xiahua Liu @xiahualiu
 * @brief PawnDB ring data structure.
 * @version 0.1
 * @date 2025-01-02
 *
 * @copyright MIT License
 *
 */

#ifndef PAWNDB_RING_H
#define PAWNDB_RING_H

#include <array>

#include "pawndb/params.h"

namespace PawnDB {

template <class T, tbl_row_t Rows>
class Ring {
  static_assert(Rows > 0, "Rows must be greater than 0!");

 public:
  struct Entry {
    T value;
    bool has_value;
  };

  constexpr Ring() noexcept : table(), head(), size() {}

  inline bool empty() const noexcept { return size == 0; }
  inline bool full() const noexcept { return size == Rows; }

  tbl_row_t get() noexcept {
    while (table[head].has_value) {
      head = (head + 1) % Rows;
    }
    auto index = head;
    table[head].has_value = true;
    head = (head + 1) % Rows;
    size++;
    return index;
  }

  void put(tbl_row_t index) noexcept {
    table[index].has_value = false;
    size--;
  }

  T& operator[](tbl_row_t index) noexcept { return table[index].value; }

 protected:
  std::array<Entry, Rows> table;
  tbl_row_t head;
  tbl_row_t size;
};

}  // namespace PawnDB

#endif  // PAWNDB_RING_H
