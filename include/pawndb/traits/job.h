#ifndef PAWNDB_TRAITS_JOB_H
#define PAWNDB_TRAITS_JOB_H

#include <sys/socket.h>

#include <cstddef>

#include "pawndb/types/buffer_table.h"

namespace PawnDB {

/**
 * @brief CRTP interface for job implementations
 * @tparam Derived The derived job class
 */
template <typename Derived>
class JobTrait {
 public:
  /** @brief Get buffer reference
   *  @return Reference to job buffer */
  BufferRef buf() const noexcept {
    return derived().trait_buf();
  }

  /** @brief Get buffer size
   *  @return Size of buffer in bytes */
  std::size_t buf_size() const noexcept {
    return derived().trait_buf_size();
  }

  /** @brief Get client address
   *  @return A pointer to client sockaddr */
  const sockaddr* c_addr() const noexcept {
    return derived().trait_c_addr();
  }

  /** @brief Get client address length
   *  @return Length of client address */
  socklen_t c_addr_len() const noexcept {
    return derived().trait_c_addr_len();
  }

 protected:
  // Protected constructor and destructor
  JobTrait() = default;
  ~JobTrait() = default;

  // CRTP helpers
  Derived& derived() noexcept {
    return static_cast<Derived&>(*this);
  }

  const Derived& derived() const noexcept {
    return static_cast<const Derived&>(*this);
  }
};

}  // namespace PawnDB

#endif  // PAWNDB_TRAITS_JOB_H
