#ifndef PAWNDB_TRAITS_JOB_H
#define PAWNDB_TRAITS_JOB_H

#include <sys/socket.h>

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
  BufferRef buffer() const noexcept {
    return static_cast<const Derived*>(this)->trait_buffer();
  }

  /** @brief Get buffer size
   *  @return Size of buffer in bytes */
  std::size_t buffer_size() const noexcept {
    return static_cast<const Derived*>(this)->trait_buffer_size();
  }

  /** @brief Get client address
   *  @return Reference to client sockaddr */
  const struct sockaddr& c_addr() const noexcept {
    return static_cast<const Derived*>(this)->trait_c_addr();
  }

  /** @brief Get client address length
   *  @return Length of client address */
  socklen_t c_addr_len() const noexcept {
    return static_cast<const Derived*>(this)->trait_c_addr_len();
  }

 protected:
  JobTrait() = default;
  ~JobTrait() = default;
};

}  // namespace PawnDB

#endif  // PAWNDB_TRAITS_JOB_H
