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
#include "pawndb/traits/copy.h"
#include "pawndb/traits/sized.h"

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
 *
 * Implemented traits:
 * - BufferManagerTrait: Buffer allocation
 * - Sized: Size tracking
 * - Container: Capacity operations
 */
class BufferTable : public BufferManagerTrait<BufferTable, BufferRef>,
                    public SizedTrait<BufferTable>,
                    public ContainerTrait<BufferTable> {
 private:
  /** @brief Fixed number of buffers in pool */
  static constexpr std::size_t N = BUFFER_ROWS;

  /**
   * @brief Buffer table entry containing buffer and metadata
   */
  struct BufferTableEntry {
    buffer_t buffer; /**< Fixed-size character buffer */
    bool is_used;    /**< Usage tracking flag */
  };

  std::array<BufferTableEntry, N> buffers_; /**< Buffer storage */
  std::size_t size_;                        /**< Current usage count */
  std::size_t next_;                        /**< Next free buffer hint */
  std::mutex mutex_;                        /**< Thread safety lock */

  friend class BufferRef; /**< Allow buffer access */

 public:
  /** @brief Initialize empty buffer pool */
  BufferTable() noexcept;

  // Not copyable
  BufferTable(const BufferTable& other) noexcept = delete;
  BufferTable& operator=(const BufferTable& other) noexcept = delete;

  // Not movable
  BufferTable(BufferTable&& other) noexcept = delete;
  BufferTable& operator=(BufferTable&& other) noexcept = delete;

  // BufferManagerTrait Implementation
  /** @brief Request new buffer allocation
   *  @return Result with buffer reference or error */
  request_r trait_request() noexcept;

  /** @brief Check if pool is empty */
  bool trait_empty() const noexcept;

  /** @brief Check if pool is full */
  bool trait_full() const noexcept;

  // Sized Implementation
  /** @brief Get count of used buffers */
  std::size_t trait_size() const noexcept;

  /** @brief Test helper to check buffer usage
   *  @param i Buffer index
   *  @return Usage flag value */
  std::uint8_t _test_is_used(std::size_t i) const noexcept;
};

/**
 * @brief Buffer reference wrapper
 */
class BufferRef : public CopyTrait<BufferRef>,
                  public BufferRefTrait<BufferRef> {
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

  /** @brief Copy constructor
   *  @param other Source reference to copy */
  BufferRef(const BufferRef& other) noexcept;

  /** @brief Copy assignment
   *  @param other Source reference to copy
   *  @return Reference to this */
  BufferRef& operator=(const BufferRef& other) noexcept;

  // Not movable
  BufferRef(BufferRef&& other) noexcept = delete;
  BufferRef& operator=(BufferRef&& other) noexcept = delete;

  // CopyTrait Implementation
  /** @brief Create clone of this reference */
  BufferRef trait_clone() const noexcept;

  /** @brief Copy from another reference
   *  @param other Source reference */
  void trait_copy(const BufferRef& other) noexcept;

  // BufferEntryTrait Implementation
  /** @brief Get underlying buffer */
  buffer_t& trait_buffer() const noexcept;

  /** @brief Release buffer back to pool */
  void trait_release() noexcept;

  /** @brief Test helper to get buffer index
   *  @return Buffer index */
  std::size_t _test_index() const noexcept;
};

}  // namespace PawnDB

#endif  // PAWNDB_TYPES_BUFFER_TABLE_H
