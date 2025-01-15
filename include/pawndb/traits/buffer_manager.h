#ifndef PAWNDB_TRAITS_BUFFER_TABLE_H
#define PAWNDB_TRAITS_BUFFER_TABLE_H

#include "pawndb/params.h"
#include "pawndb/result.h"

namespace PawnDB {

/**
 * @brief CRTP base class for buffer entries providing buffer access and
 * lifecycle management
 * @tparam Derived The derived buffer entry class implementing required traits
 *
 * Required implementations:
 * - trait_buffer() -> buffer_t& : Access underlying buffer
 * - trait_release() -> void : Release buffer resources
 */
template <typename Derived>
class BufferRefTrait {
 public:
  /** @brief Get reference to underlying buffer */
  buffer_t& buffer() const noexcept {
    return static_cast<const Derived*>(this)->trait_buffer();
  }

  /** @brief Release buffer resources */
  void release() noexcept { static_cast<Derived*>(this)->trait_release(); }

  /** @brief Check if buffer is null */
  bool null() const noexcept {
    return static_cast<const Derived*>(this)->trait_null();
  }

 protected:
  BufferRefTrait() = default;
  ~BufferRefTrait() = default;
};

/** @brief Result type for buffer requests */
enum class BufferError {
  None, /**< Operation successful */
  Full, /**< No free buffers */
};

/**
 * @brief CRTP base class for buffer managers providing buffer allocation and
 * management
 * @tparam Derived The derived buffer manager class
 * @tparam BufferRefType The buffer entry type implementing BufferRefTrait
 *
 * Required implementations:
 * - trait_request() -> RequestR : Allocate new buffer entry
 */
template <typename Derived, typename BufferRefType>
class BufferManagerTrait {
 public:
  /** @brief Request result type */
  using request_r = Result<BufferRefType&, BufferError>;

  /** @brief Request new buffer allocation */
  request_r request() noexcept {
    return static_cast<Derived*>(this)->trait_request();
  }

 protected:
  BufferManagerTrait() = default;
  ~BufferManagerTrait() = default;
};

}  // namespace PawnDB

#endif  // PAWNDB_TRAITS_BUFFER_TABLE_H
