/**
 * @file queue.h
 * @author Xiahua Liu @xiahualiu
 * @brief PawnDB queue data structure.
 * @version 0.1
 * @date 2025-01-02
 *
 * @copyright MIT License
 *
 */

#ifndef PAWNDB_QUEUE_H
#define PAWNDB_QUEUE_H

#include <array>

#include "pawndb/params.h"

namespace PawnDB {

enum class QueueError {
  None,
  Empty,
  Full,
};

template <class T, tbl_row_t Rows>
class Queue {
  static_assert(Rows > 0, "Rows must be greater than 0!");

 public:
  constexpr Queue() noexcept : table(), head(), tail(), size() {}

  inline bool empty() const noexcept { return size == 0; }
  inline bool full() const noexcept { return size == Rows; }

  inline void push(const T& _value) noexcept {
    table[tail] = _value;
    tail = (tail + 1) % Rows;
    size++;
  }

  inline void push(const T&& _value) noexcept {
    table[tail] = _value;
    tail = (tail + 1) % Rows;
    size++;
  }

  inline void pop() noexcept {
    head = (head + 1) % Rows;
    size--;
  }

  inline T& front() noexcept { return table[head]; }

  inline void clear() noexcept {
    head = 0;
    tail = 0;
    size = 0;
  }

 protected:
  std::array<T, Rows> table;
  tbl_row_t head;
  tbl_row_t tail;
  tbl_row_t size;
};

}  // namespace PawnDB

#endif  // PAWNDB_QUEUE_H
