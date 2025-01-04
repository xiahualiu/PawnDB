/**
 * @file buffer_table.h
 * @brief Buffer pool management for PawnDB
 * @version 0.1
 * @date 2025-01-02
 *
 * This file implements a buffer pool manager that:
 * - Manages fixed-size buffers
 * - Provides reference counting
 * - Ensures thread-safe access
 * - Uses RAII for automatic cleanup
 *
 * @copyright MIT License
 */

#ifndef PAWNDB_TABLE_BUFFER_H
#define PAWNDB_TABLE_BUFFER_H

#include <array>
#include <mutex>

#include "pawndb/params.h"

namespace PawnDB {

/**
 * @brief Buffer pool manager class
 *
 * Manages a pool of fixed-size buffers with:
 * - Reference counting
 * - Thread-safe access
 * - Automatic buffer recycling
 */
class BufferTable {
 private:
 public:
  /** @brief Type alias for buffer storage */
  using buffer_t = std::array<char, BUFFER_WIDTH>;

  /**
   * @brief RAII wrapper for buffer access
   *
   * Provides:
   * - Automatic reference counting
   * - Move semantics
   * - Safe buffer access
   */
  class BufferRef {
   private:
    BufferTable* table;
    tbl_row_t index;

   public:
    BufferRef() : table(nullptr), index() {}

    /**
     * @brief Constructs a new buffer reference.
     * @note Table must not be null.
     * @param _table Buffer pool pointer
     * @param _index Buffer index
     */
    BufferRef(BufferTable* _table, tbl_row_t _index)
        : table(_table), index(_index) {
      table->ref_counts[index] = 1;
    }

    /**
     * @brief Move constructor for BufferRef.
     *
     * This constructor initializes a BufferRef object by transferring ownership
     * of resources from another BufferRef object.
     *
     * @param _other The BufferRef object to move from. After the move, _other
     *               will be in a valid but unspecified state.
     */
    BufferRef(BufferRef&& _other) noexcept
        : table(_other.table), index(_other.index) {
      _other.table = nullptr;
    }

    /**
     * @brief Copy constructor for BufferRef.
     *
     * This constructor initializes a BufferRef object by copying the contents
     * of another BufferRef object. It increments the reference count of the
     * buffer being copied.
     *
     * @param _other The BufferRef object to copy from.
     */
    BufferRef(const BufferRef& _other) noexcept
        : table(_other.table), index(_other.index) {
      table->ref_counts[index]++;
    }

    /**
     * @brief Copy assignment operator for BufferRef.
     *
     * This operator assigns the values from another BufferRef instance to this
     * instance. It copies the table and index from the other instance and
     * increments the reference count for the buffer at the specified index in
     * the table.
     *
     * @param _other The BufferRef instance to copy from.
     * @return A reference to this BufferRef instance.
     */
    BufferRef& operator=(const BufferRef& _other) noexcept {
      if (this->table != nullptr) {
        std::lock_guard<std::mutex> lock(this->table->table_mutex);
        this->table->ref_counts[this->index]--;
      }
      this->table = _other.table;
      this->index = _other.index;
      if (this->table != nullptr) {
        table->ref_counts[index]++;
      }
      return *this;
    }

    /**
     * @brief Move assignment operator for BufferRef.
     *
     * This operator assigns the values from another BufferRef instance to this
     * instance. It moves the table and index from the other instance.
     *
     * @param _other The BufferRef instance to move from.
     * @return A reference to this BufferRef instance.
     */
    BufferRef& operator=(BufferRef&& _other) noexcept {
      if (this->table != nullptr) {
        std::lock_guard<std::mutex> lock(this->table->table_mutex);
        this->table->ref_counts[this->index]--;
      }
      this->table = _other.table;
      this->index = _other.index;
      _other.table = nullptr;
      return *this;
    }

    /**
     * @brief Destructor for BufferRef.
     *
     * This destructor decrements the reference count of the buffer
     * associated with this BufferRef object.
     */
    ~BufferRef() {
      if (table != nullptr) {
        std::lock_guard<std::mutex> lock(table->table_mutex);
        table->ref_counts[index]--;
      }
    }

    /**
     * @brief Get reference to buffer
     */
    buffer_t& operator*() { return table->buffers[index]; }

    /**
     * @brief Checks if the reference is null.
     *
     * This operator returns true if the table is a null pointer.
     *
     * @return true if the table is null, false otherwise.
     */
    bool operator!() const noexcept { return table == nullptr; }

    /**
     * @brief Checks if the reference is not null.
     *
     * This operator returns true if the table is not a null pointer.
     *
     * @return true if the table is not null, false otherwise.
     */
    explicit operator bool() const noexcept { return table != nullptr; }

    /**
     * @brief Get index of buffer
     * @note Test function for unit tests only.
     */
    tbl_row_t _test_index() { return index; }
  };

  /**
   * @brief Default constructs empty buffer pool
   */
  BufferTable() noexcept : buffers(), ref_counts(), head() {}

  /**
   * @brief Requests a buffer from the pool
   * @return Reference-counted buffer wrapper
   */
  BufferRef request() noexcept {
    std::lock_guard<std::mutex> lock(table_mutex);
    while (ref_counts[head] > 0) {
      head = (head + 1) % BUFFER_ROWS;
    }
    return BufferRef(this, head);
  }

  /**
   * @brief Returns the reference count of a buffer.
   * @note Test function for unit tests only.
   * @param _index The index of the buffer to check.
   * @return The reference count of the buffer at the given index.
   */
  tbl_row_t _test_counter(tbl_row_t _index) { return ref_counts[_index]; }

 private:
  std::mutex table_mutex;
  std::array<std::array<char, BUFFER_WIDTH>, BUFFER_ROWS> buffers;
  std::array<tbl_row_t, BUFFER_ROWS> ref_counts;
  tbl_row_t head;
};

}  // namespace PawnDB

#endif  // PAWNDB_TABLE_BUFFER_H
