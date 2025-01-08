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
#include "pawndb/traits/buffer_manager.h"
#include "pawndb/traits/container.h"
#include "pawndb/traits/sized.h"

namespace PawnDB {

using buffer_t = std::array<char, BUFFER_WIDTH>;

/**
 * @brief Fixed-size thread-safe buffer pool
 *
 * Manages a pool of fixed-size character buffers with:
 * - Thread-safe allocation/deallocation
 * - Usage tracking
 * - Bounds checking
 * - Size monitoring
 */
class BufferTable : public BufferManagerTrait<BufferTable, buffer_t>,
                    public Sized<BufferTable>,
                    public Container<BufferTable> {
  static constexpr std::size_t N = BUFFER_ROWS;

 private:
  struct BufferEntry {
    std::array<char, BUFFER_WIDTH> buffer; /**< Fixed-size buffer */
    bool is_used;                          /**< Usage tracking */
  };

  std::array<BufferEntry, N> buffers_; /**< Buffer storage */
  std::size_t size_;                   /**< Current usage count */
  std::size_t next_;                   /**< Next free buffer hint */
  std::mutex mutex_;                   /**< Thread safety lock */

 public:
  BufferTable() noexcept : buffers_(), size_(0), next_(0) {}

  /**
   * @brief Request a free buffer
   * @return Success: buffer index, Error: BufferError
   */
  RequestR trait_request() noexcept {
    if (trait_full()) {
      return BufferError::Full;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    while (buffers_[next_].is_used) {
      next_ = (next_ + 1) % N;
    }
    buffers_[next_].is_used = true;
    size_++;
    return next_++;
  }

  /**
   * @brief Release a buffer
   * @param index Buffer to release
   * @return BufferError None or error code
   */
  RequestR trait_release(std::size_t index) noexcept {
    if (index >= N) return BufferError::OutOfRange;
    std::lock_guard<std::mutex> lock(mutex_);
    if (buffers_[index].is_used) {
      buffers_[index].is_used = false;
      size_--;
      return BufferError::None;
    } else {
      return BufferError::NotUsed;
    }
  }

  /**
   * @brief Access buffer by index
   * @param index Buffer index
   * @return Reference to buffer
   */
  buffer_t& trait_get(std::size_t index) noexcept {
    return buffers_[index].buffer;
  }

  bool trait_empty() const noexcept { return size_ == 0; }
  bool trait_full() const noexcept { return size_ >= N; }
  std::size_t trait_size() const noexcept { return size_; }
  constexpr std::size_t trait_capacity() const noexcept { return N; }
};

}  // namespace PawnDB

#endif  // PAWNDB_TYPES_BUFFER_TABLE_H
