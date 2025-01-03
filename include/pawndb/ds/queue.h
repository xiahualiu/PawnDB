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

/**
 * @brief Enumeration of possible queue errors.
 */
enum class QueueError {
  None,  /**< No error */
  Empty, /**< Queue is empty */
  Full,  /**< Queue is full */
};

/**
 * @brief Queue data structure.
 *
 * @tparam T The type of the elements in the queue.
 * @tparam Rows The number of rows in the queue.
 */
template <class T, tbl_row_t Rows>
class Queue {
  static_assert(Rows > 0, "Rows must be greater than 0!");

 public:
  /**
   * @brief Constructs a new Queue object.
   */
  constexpr Queue() noexcept : table(), head(), tail(), size() {}

  /**
   * @brief Checks if the queue is empty.
   *
   * @return true if the queue is empty, false otherwise.
   */
  inline bool empty() const noexcept { return size == 0; }

  /**
   * @brief Checks if the queue is full.
   *
   * @return true if the queue is full, false otherwise.
   */
  inline bool full() const noexcept { return size == Rows; }

  /**
   * @brief Pushes an element into the queue.
   *
   * @param _value The value to push.
   */
  inline void push(const T& _value) noexcept {
    table[tail] = _value;
    tail = (tail + 1) % Rows;
    size++;
  }

  /**
   * @brief Pushes an element into the queue.
   *
   * @param _value The value to push.
   */
  inline void push(const T&& _value) noexcept {
    table[tail] = _value;
    tail = (tail + 1) % Rows;
    size++;
  }

  /**
   * @brief Pops an element from the queue.
   */
  inline void pop() noexcept {
    head = (head + 1) % Rows;
    size--;
  }

  /**
   * @brief Returns a reference to the front element of the queue.
   *
   * This function provides access to the element at the front of the queue
   * without removing it. The queue must not be empty when this function is
   * called.
   *
   * @return T& Reference to the front element of the queue.
   */
  inline T& front() noexcept { return table[head]; }

  /**
   * @brief Clears the queue by resetting the head, tail, and size to zero.
   *
   * This function sets the head, tail, and size of the queue to zero,
   * effectively removing all elements from the queue.
   */
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
