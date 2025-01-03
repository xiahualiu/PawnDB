/**
 * @file hash.h
 * @author Xiahua Liu @xiahualiu
 * @brief PawnDB buffer table, implemented as a ring buffer. Note there is no
 * safety check for buffer overflow. So the buffer size should be big enough.
 * @version 0.1
 * @date 2025-01-02
 *
 * @copyright MIT License
 *
 */

#ifndef PAWNDB_TABLE_BUFFER_H
#define PAWNDB_TABLE_BUFFER_H

#include <array>
#include <mutex>

#include "pawndb/ds/ring.h"
#include "pawndb/params.h"

namespace PawnDB {

/**
 * @brief Class representing the buffer table.
 *
 * The buffer table is implemented as a ring buffer. Note that there is no
 * safety check for buffer overflow, so the buffer size should be big enough.
 */
class BufferTable : private Ring<std::array<char, BUFFER_WIDTH>, BUFFER_ROWS> {
 private:
  using _base = Ring<std::array<char, BUFFER_WIDTH>, BUFFER_ROWS>;

 public:
  using buffer_t = std::array<char, BUFFER_WIDTH>;

  /**
   * @brief Constructs a new BufferTable object.
   */
  BufferTable() noexcept : _base(), table_mutex() {}

  /**
   * @brief Requests a buffer from the buffer table.
   *
   * @return The index of the requested buffer.
   */
  tbl_row_t request() noexcept {
    std::lock_guard<std::mutex> lock(table_mutex);
    return _base::get();
  }

  /**
   * @brief Releases a buffer back to the buffer table.
   *
   * @param index The index of the buffer to release.
   */
  void release(tbl_row_t index) noexcept {
    std::lock_guard<std::mutex> lock(table_mutex);
    _base::put(index);
  }

  /**
   * @brief Accesses the buffer at the specified index.
   *
   * @param index The index of the buffer to access.
   * @return A reference to the buffer at the specified index.
   */
  buffer_t& operator[](tbl_row_t index) noexcept {
    return _base::operator[](index);
  }

 private:
  std::mutex table_mutex;
};

}  // namespace PawnDB

#endif  // PAWNDB_TABLE_BUFFER_H
