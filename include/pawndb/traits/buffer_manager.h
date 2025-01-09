/**
 * @file buffer_table.h
 * @brief CRTP interface for buffer pool management
 * @version 0.1
 * @date 2025-01-02
 *
 * Features:
 * - Buffer allocation/deallocation
 * - Index-based access
 * - Thread safety support
 */
#ifndef PAWNDB_TRAITS_BUFFER_TABLE_H
#define PAWNDB_TRAITS_BUFFER_TABLE_H

#include <cstddef>

#include "pawndb/result.h"

namespace PawnDB {

/**
 * @brief CRTP interface for buffer table implementations
 * @tparam Derived Class implementing buffer interface
 * @tparam BufferType Type of buffer stored in table
 *
 * Required implementations:
 * - size_t trait_request()
 * - void trait_release(size_t)
 * - BufferType& trait_get(size_t)
 */
template <typename Derived, typename BufferType>
class BufferManagerTrait {
 public:
  /**
   * @brief Buffer operation error codes
   */
  enum class BufferError {
    None,       /**< Operation successful */
    Full,       /**< No free buffers */
    OutOfRange, /**< Invalid buffer index */
    NotUsed     /**< Buffer not allocated */
  };

  using RequestR = Result<std::size_t, BufferError>;

  RequestR request() noexcept {
    return static_cast<Derived*>(this)->trait_request();
  }

  /**
   * @brief Release buffer back to pool
   * @param index Index of buffer to release
   */
  RequestR release(std::size_t index) noexcept {
    return static_cast<Derived*>(this)->trait_release(index);
  }

  /**
   * @brief Access buffer by index
   * @param index Buffer index
   * @return Reference to buffer
   */
  BufferType& operator[](std::size_t index) noexcept {
    return static_cast<Derived*>(this)->trait_get(index);
  }

 protected:
  BufferManagerTrait() = default;
  ~BufferManagerTrait() = default;
};

}  // namespace PawnDB

#endif  // PAWNDB_TRAITS_BUFFER_TABLE_H
