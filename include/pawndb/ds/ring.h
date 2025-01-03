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

/**
 * @brief Ring buffer data structure.
 *
 * @tparam T The type of the elements in the ring buffer.
 * @tparam Rows The number of rows in the ring buffer.
 */
template <class T, tbl_row_t Rows>
class Ring {
  static_assert(Rows > 0, "Rows must be greater than 0!");

 public:
  /**
   * @brief Structure representing an entry in the ring buffer.
   */
  struct Entry {
    T value;        /**< The value of the entry */
    bool has_value; /**< Flag indicating if the entry has a value */
  };

  /**
   * @brief Constructs a new Ring object.
   */
  constexpr Ring() noexcept : table(), head(), size() {}

  /**
   * @brief Checks if the ring buffer is empty.
   *
   * @return true if the ring buffer is empty, false otherwise.
   */
  inline bool empty() const noexcept { return size == 0; }

  /**
   * @brief Checks if the ring buffer is full.
   *
   * @return true if the ring buffer is full, false otherwise.
   */
  inline bool full() const noexcept { return size == Rows; }

  /**
   * @brief Gets the next available index in the ring buffer.
   *
   * @return The index of the next available entry.
   */
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

  /**
   * @brief Marks an entry as available.
   *
   * @param index The index of the entry to mark as available.
   */
  void put(tbl_row_t index) noexcept {
    table[index].has_value = false;
    size--;
  }

  /**
   * @brief Accesses the value at the specified index.
   *
   * @param index The index of the entry to access.
   * @return A reference to the value at the specified index.
   */
  T& operator[](tbl_row_t index) noexcept { return table[index].value; }

 protected:
  std::array<Entry, Rows> table;
  tbl_row_t head;
  tbl_row_t size;
};

}  // namespace PawnDB

#endif  // PAWNDB_RING_H
