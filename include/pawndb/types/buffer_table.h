/**
 * @file buffer_table.h
 * @brief Fixed-size thread-safe buffer pool implementation
 * @version 0.1
 * @date 2025-01-02
 *
 * Features:
 * - Fixed buffer allocation
 * - Thread safety
 * - Size tracking
 * - Error handling
 */
#ifndef PAWNDB_TYPES_BUFFER_TABLE_H
#define PAWNDB_TYPES_BUFFER_TABLE_H

#include <array>
#include <cstddef>
#include <mutex>

#include "pawndb/params.h"

namespace PawnDB {

class BufferRef;

/**
 * @brief Fixed-size thread-safe buffer pool
 *
 * Features:
 * - Thread-safe buffer allocation/deallocation
 * - Reference counting and usage tracking
 * - Fixed-size buffer storage (BUFFER_ROWS * BUFFER_SIZE)
 * - O(1) allocation via next-fit strategy
 */
class BufferTable {
 public:
  /** @brief Result type for request operations */
  using request_r = Result<BufferRef, BufferError>;

 private:
  /** @brief Fixed number of buffers in pool */
  static constexpr std::size_t N = BUFFER_ROWS;

  /**
   * @brief Buffer table entry containing buffer and metadata
   */
  struct BufferTableEntry {
    buffer_t buffer_;         /**< Fixed-size character buffer */
    std::uint16_t ref_count_; /**< Reference count for this slot */

    /** @brief Default constructor */
    constexpr BufferTableEntry() noexcept : buffer_{}, ref_count_(0) {}
  };

  std::array<BufferTableEntry, N> buffers_; /**< Buffer storage */
  std::size_t size_;                        /**< Current usage count */
  std::size_t next_;                        /**< Next free buffer hint */
  mutable std::mutex mutex_;                /**< Thread safety lock */

  friend class BufferRef; /**< Allow buffer access */

  /* Retain a reference to buffer at index (increment ref count) */
  void retain_ref_(std::size_t index) noexcept;

  /* Release a reference to buffer at index (decrement ref count) */
  void release_ref_(std::size_t index) noexcept;

 public:
  /** @brief Initialize empty buffer pool */
  constexpr BufferTable() noexcept : buffers_{}, size_(0), next_(0) {}

  // Not copyable
  BufferTable(const BufferTable& other) noexcept = delete;
  BufferTable& operator=(const BufferTable& other) noexcept = delete;

  /** @brief Request new buffer allocation
   *  @return Result with buffer reference or error */
  request_r request() noexcept;

  /** @brief Release all buffers back to pool */
  void clear_() noexcept;

  /** @brief Check if pool is empty */
  bool empty() const noexcept;

  /** @brief Check if pool is full */
  bool full() const noexcept;

  /** @brief Get count of used buffers */
  std::size_t size() const noexcept;

  /** @brief Get buffer reference at index */
  BufferRef get_(std::size_t idx) noexcept;

  /** @brief Zero out buffer at index */
  void zero_(std::size_t idx) noexcept;

  /** @brief Test helper to check buffer usage
   *  @param i Buffer index
   *  @return Usage flag value */
  std::uint8_t _test_is_used(std::size_t i) const noexcept;

  /** @brief Test helper to check raw reference count
   *  @param i Buffer index
   *  @return Current ref count */
  std::uint16_t _test_ref_count(std::size_t i) const noexcept;
};

/**
 * @brief Buffer reference wrapper
 */
class BufferRef {
 private:
  BufferTable* table_; /**< Owner table reference */
  std::size_t index_;  /**< Buffer index */

 public:
  /** @brief Default constructor - creates invalid reference */
  constexpr BufferRef() noexcept : table_(nullptr), index_(0) {}

  /** @brief Constructor with table and index
   *  @param table Owner buffer table
   *  @param index Buffer index */
  BufferRef(BufferTable* table, std::size_t index) noexcept;

  // Copyable
  BufferRef(const BufferRef& other) noexcept;
  BufferRef& operator=(const BufferRef& other) noexcept;

  // Movable
  BufferRef(BufferRef&& other) noexcept;
  BufferRef& operator=(BufferRef&& other) noexcept;
  ~BufferRef() noexcept;

  /** @brief Create a value copy of this reference */
  BufferRef copy() const noexcept;

  /** @brief Copy from another reference
   *  @param other Source reference */
  void copy_from(const BufferRef& other) noexcept;

  /** @brief Create new object by moving from this reference */
  BufferRef move() noexcept;

  /** @brief Move from another reference
   *  @param other Source rvalue reference */
  void move_from(BufferRef&& other) noexcept;

  /** @brief Check if reference is null
   *  @return true if reference is invalid */
  bool _test_null() const noexcept;

  /** @brief Get underlying buffer */
  buffer_t& buffer() const noexcept;

  /** @brief Test helper to get buffer index
   *  @return Buffer index */
  std::size_t _test_index() const noexcept;

 private:
  void release_ref_() noexcept;
};

}  // namespace PawnDB

#endif  // PAWNDB_TYPES_BUFFER_TABLE_H