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

class BufferTable : private Ring<std::array<char, BUFFER_WIDTH>, BUFFER_ROWS> {
 private:
  using _base = Ring<std::array<char, BUFFER_WIDTH>, BUFFER_ROWS>;

 public:
  using buffer_t = std::array<char, BUFFER_WIDTH>;

  BufferTable() noexcept : _base(), table_mutex() {}

  tbl_row_t request() noexcept {
    auto lock = std::lock_guard<std::mutex>(table_mutex);
    return _base::get();
  }

  void release(tbl_row_t row) noexcept {
    auto lock = std::lock_guard<std::mutex>(table_mutex);
    _base::put(row);
  }

  using _base::operator[];

 private:
  std::mutex table_mutex;
};

}  // namespace PawnDB

#endif  // PAWNDB_TABLE_BUFFER_H
