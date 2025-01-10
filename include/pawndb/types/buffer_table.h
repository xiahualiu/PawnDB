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

namespace PawnDB {

class BufferRC;

/**
 * @brief Fixed-size thread-safe buffer pool
 *
 * Manages a pool of fixed-size character buffers with:
 * - Thread-safe allocation/deallocation
 * - Usage tracking
 * - Bounds checking
 * - Size monitoring
 */
class BufferTable : public BufferManagerTrait<BufferTable, BufferRC>,
                    public Container<BufferTable> {
  static constexpr std::size_t N = BUFFER_ROWS;

 private:
  struct BufferTableEntry {
    buffer_t buffer;        /**< Fixed-size buffer */
    std::uint8_t ref_count; /**< Usage tracking */
  };

  std::array<BufferTableEntry, N> buffers_; /**< Buffer storage */
  std::size_t size_;                        /**< Current usage count */
  std::size_t next_;                        /**< Next free buffer hint */
  std::mutex mutex_;                        /**< Thread safety lock */

  friend class BufferRC;

 public:
  BufferTable() noexcept : buffers_(), size_(0), next_(0) {}

  // BufferManagerTrait
  RequestR trait_request() noexcept;
  // Container
  bool trait_empty() const noexcept { return size_ == 0; }
  bool trait_full() const noexcept { return size_ >= N; }
  constexpr std::size_t trait_capacity() const noexcept { return N; }
};

class BufferRC {
 private:
  BufferTable* table_;
  std::size_t index_;

 public:
  BufferRC() = default;

  BufferRC(BufferTable* _table, std::size_t _index) noexcept
      : table_(_table), index_(_index) {}

  ~BufferRC() noexcept {
    if (table_ == nullptr) return;
    std::lock_guard<std::mutex> lock(table_->mutex_);
    table_->buffers_[index_].ref_count--;
    if (table_->buffers_[index_].ref_count == 0) {
      table_->size_--;
    }
  }

  BufferRC(const BufferRC& _other) noexcept
      : table_(_other.table_), index_(_other.index_) {
    if (table_ == nullptr) return;
    std::lock_guard<std::mutex> lock(table_->mutex_);
    table_->buffers_[index_].ref_count++;
  }

  BufferRC(BufferRC&& _other) noexcept
      : table_(_other.table_), index_(_other.index_) {
    _other.table_ = nullptr;
    _other.index_ = 0;
  }

  BufferRC& operator=(const BufferRC& _other) noexcept {
    // Release the current stored buffer
    if (table_ != nullptr) {
      std::lock_guard<std::mutex> lock(table_->mutex_);
      table_->buffers_[_other.index_].ref_count--;
    }
    // Copy the other buffer
    table_ = _other.table_;
    index_ = _other.index_;
    // Increase the reference count, since we are now sharing the buffer
    if (table_ != nullptr) {
      std::lock_guard<std::mutex> lock(table_->mutex_);
      table_->buffers_[index_].ref_count++;
    }
    return *this;
  }

  BufferRC& operator=(BufferRC&& _other) noexcept {
    // Release the current stored buffer
    if (table_ != nullptr) {
      std::lock_guard<std::mutex> lock(table_->mutex_);
      table_->buffers_[index_].ref_count--;
    }
    // Move the other buffer
    table_ = _other.table_;
    index_ = _other.index_;
    _other.table_ = nullptr;
    _other.index_ = 0;
    return *this;
  }

  buffer_t& to_array() noexcept { return table_->buffers_[index_].buffer; }

  const buffer_t& to_array() const noexcept {
    return table_->buffers_[index_].buffer;
  }
};

}  // namespace PawnDB

#endif  // PAWNDB_TYPES_BUFFER_TABLE_H
