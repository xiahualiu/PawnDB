#ifndef PAWNDB_TYPES_BUFFER_TABLE_H
#define PAWNDB_TYPES_BUFFER_TABLE_H

#include <array>
#include <cstddef>
#include <mutex>

#include "pawndb/params.h"
#include "pawndb/result.h"

namespace PawnDB {

class BufferTable;

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

  /** @brief Get underlying buffer */
  buffer_t& data() const noexcept;

  /** @brief Release buffer back to pool */
  void release() noexcept;

  /** @brief Test helper to get buffer index
   *  @return Buffer index */
  std::size_t _test_index() const noexcept;

  /** @brief Check if reference is null
   *  @return true if reference is invalid */
  bool _test_null() const noexcept;
};

/**
 * @brief Fixed-size thread-safe buffer pool
 *
 * Features:
 * - Thread-safe buffer allocation/deallocation
 * - Fixed-size buffer storage (BUFFER_ROWS * BUFFER_SIZE)
 * - O(1) allocation via next-fit strategy
 */
class BufferTable {
 private:
  /** @brief Fixed number of buffers in pool */
  static constexpr std::size_t N = BUFFER_ROWS;

  /**
   * @brief Buffer table entry containing buffer and metadata
   */
  struct BufferTableEntry {
    buffer_t buffer_; /**< Fixed-size character buffer */
    bool is_used_;    /**< Usage tracking flag */

    /** @brief Default constructor */
    constexpr BufferTableEntry() noexcept : buffer_{}, is_used_(false) {}
  };

  std::array<BufferTableEntry, N> buffers_; /**< Buffer storage */
  std::size_t size_;                        /**< Current usage count */
  std::size_t next_;                        /**< Next free buffer hint */
  std::mutex mutex_;                        /**< Thread safety lock */

  friend class BufferRef; /**< Allow buffer access */

 public:
  /** @brief Result type for buffer requests */
  enum class BufferError {
    None, /**< Operation successful */
    Full, /**< No free buffers */
  };

  /** @brief Request result type */
  using buf_req_r = Result<BufferRef, BufferError>;

  /** @brief Initialize empty buffer pool */
  constexpr BufferTable() noexcept : buffers_{}, size_(0), next_(0) {}

  // Not copyable
  BufferTable(const BufferTable& other) noexcept = delete;
  BufferTable& operator=(const BufferTable& other) noexcept = delete;

  /** @brief Request new buffer allocation
   *  @return Result with buffer reference or error */
  buf_req_r request() noexcept;

  /**  @brief Release all buffers back to pool */
  void clear() noexcept;

  /** @brief Check if pool is full */
  bool full() const noexcept;

  /** @brief Get count of used buffers */
  std::size_t size() const noexcept;

  /** @brief Test helper to check buffer usage
   *  @param i Buffer index
   *  @return Usage flag value */
  std::uint8_t _test_is_used(std::size_t i) const noexcept;
};

}  // namespace PawnDB

#endif  // PAWNDB_TYPES_BUFFER_TABLE_H
