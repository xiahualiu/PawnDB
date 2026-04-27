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
#include "pawndb/result.h"

namespace PawnDB {

class buf_ref;

/** @brief Buffer operation error codes */
enum class BufferError {
  None, /**< Operation successful */
  Full  /**< Buffer pool full */
};

/**
 * @brief Fixed-size thread-safe buffer pool
 *
 * Features:
 * - Thread-safe buffer allocation/deallocation
 * - Reference counting and usage tracking
 * - Fixed-size buffer storage (BUFFER_ROWS * BUFFER_SIZE)
 * - O(1) allocation via next-fit strategy
 */
class buf_table {
 public:
  /** @brief Result type for request operations */
  using buf_req_r = Result<buf_ref, BufferError>;

 private:
  /** @brief Buffer table entry containing buffer and metadata */
  struct buf_table_entry {
    buf_t buffer_;            /**< Fixed-size character buffer */
    std::uint16_t ref_count_; /**< Reference count for this slot */

    /** @brief Default constructor */
    constexpr buf_table_entry() noexcept : buffer_{}, ref_count_(0) {}
  };

  std::array<buf_table_entry, BUFFER_ROWS> buffers_; /**< Buffer storage */
  std::size_t size_;                                 /**< Current usage count */
  std::size_t next_;         /**< Next free buffer hint */
  mutable std::mutex mutex_; /**< Thread safety lock */

  friend class buf_ref; /**< Allow buffer access */

  /* Retain a reference to buffer at index (increment ref count) */
  void retain_ref(std::size_t index) noexcept;

  /* Release a reference to buffer at index (decrement ref count) */
  void release_ref(std::size_t index) noexcept;

 public:
  /** @brief Initialize empty buffer pool */
  constexpr buf_table() noexcept : buffers_{}, size_(0), next_(0) {}

  // Not copyable
  buf_table(const buf_table& other) noexcept = delete;
  buf_table& operator=(const buf_table& other) noexcept = delete;

  /** @brief Request new buffer allocation
   *  @return Result with buffer reference or error */
  buf_req_r request() noexcept;

  /** @brief Check if pool is empty */
  bool empty() const noexcept;

  /** @brief Check if pool is full */
  bool full() const noexcept;

  /** @brief Get count of used buffers */
  std::size_t size() const noexcept;

  friend std::uint8_t test_is_used(const buf_table& t, std::size_t i);
  friend std::uint16_t test_ref_count(const buf_table& t, std::size_t i);
};

/**
 * @brief Buffer reference wrapper
 */
class buf_ref {
 private:
  buf_table* table_;  /**< Owner table reference */
  std::size_t index_; /**< Buffer index */

 public:
  /** @brief Default constructor - creates invalid reference */
  constexpr buf_ref() noexcept : table_(nullptr), index_(0) {}

  /** @brief Constructor with table and index
   *  @param table Owner buffer table
   *  @param index Buffer index */
  buf_ref(buf_table* table, std::size_t index) noexcept;

  // Copyable
  buf_ref(const buf_ref& other) noexcept;
  buf_ref& operator=(const buf_ref& other) noexcept;

  // Movable
  buf_ref(buf_ref&& other) noexcept;
  buf_ref& operator=(buf_ref&& other) noexcept;
  ~buf_ref() noexcept;

  friend bool test_null(const buf_ref& ref);
  friend std::size_t test_index(const buf_ref& ref);

  /** @brief Get underlying buffer */
  buf_t& buffer() const noexcept;

 private:
  void release_ref_() noexcept;
};

}  // namespace PawnDB

#endif  // PAWNDB_TYPES_BUFFER_TABLE_H
