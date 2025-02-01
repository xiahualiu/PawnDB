#ifndef PAWNDB_TRAITS_BUFFER_MANAGER_H
#define PAWNDB_TRAITS_BUFFER_MANAGER_H

#include "pawndb/params.h"
#include "pawndb/result.h"

namespace PawnDB {

/**
 * @brief CRTP base class for buffer entries providing access to underlying
 * buffer
 * @tparam Derived The derived buffer entry class
 *
 * Required implementations:
 * - trait_buf() -> buffer_t& : Get reference to underlying buffer
 * - trait_release() : Release buffer resources
 */
template <typename Derived>
class BufferRefTrait {
 public:
  /** @brief Get reference to underlying buffer */
  buffer_t& buf() const noexcept {
    return derived().trait_buf();
  }

  /** @brief Release buffer resources */
  void release() noexcept {
    derived().trait_release();
  }

 private:
  // CRTP helpers
  Derived& derived() {
    return static_cast<Derived&>(*this);
  }
  const Derived& derived() const {
    return static_cast<const Derived&>(*this);
  }
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
  using request_r = Result<BufferRefType, BufferError>;

  /** @brief Request new buffer allocation */
  request_r request() noexcept {
    return derived().trait_request();
  }

 protected:
  // Hide constructor
  BufferManagerTrait() = default;
  ~BufferManagerTrait() = default;

  // CRTP helpers
  Derived& derived() {
    return static_cast<Derived&>(*this);
  }
  const Derived& derived() const {
    return static_cast<const Derived&>(*this);
  }
};

}  // namespace PawnDB

#endif  // PAWNDB_TRAITS_BUFFER_MANAGER_H
