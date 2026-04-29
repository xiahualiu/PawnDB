#ifndef PAWNDB_TYPES_BUF_TABLE_H
#define PAWNDB_TYPES_BUF_TABLE_H

#include <array>
#include <cstddef>
#include <mutex>

#include "pawndb/params.h"
#include "pawndb/result.h"
#include "pawndb/types/buf_ref.h"

namespace PawnDB {

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

}  // namespace PawnDB

#endif  // PAWNDB_TYPES_BUF_TABLE_H
