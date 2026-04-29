#ifndef PAWNDB_TYPES_JOB_H
#define PAWNDB_TYPES_JOB_H

#include <cstddef>

#include "pawndb/types/client_conn.h"
#include "pawndb/types/job_buf.h"

namespace PawnDB {

/**
 * @brief Database job containing client request data
 *
 * Owns a job_buf (buffer + protocol parsing) and tracks client connection.
 * Copy operations correctly manage buffer reference counting.
 */
class job {
 public:
  /** @brief Initialize empty job */
  constexpr job() noexcept : buf_(), conn_() {}

  /** @brief Initialize job, moving buf_ref into internal job_buf */
  job(buf_ref&& _buffer, std::size_t _buffer_size, client_conn _conn) noexcept;

  // Copyable
  job(const job& other) noexcept;
  job& operator=(const job& other) noexcept;

  /** @brief Get mutable job buffer (parsing + buffer access) */
  job_buf& buf() noexcept;

  /** @brief Get const job buffer */
  const job_buf& buf() const noexcept;

  /** @brief Get buffer size */
  std::size_t buf_sz() const noexcept;

  /** @brief Get client connection */
  const client_conn& conn() const noexcept;

 private:
  job_buf buf_;      /**< Job buffer with parsing */
  client_conn conn_; /**< Client connection descriptor */
};

}  // namespace PawnDB

#endif
