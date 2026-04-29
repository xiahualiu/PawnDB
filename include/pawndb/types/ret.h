#ifndef PAWNDB_TYPES_RET_H
#define PAWNDB_TYPES_RET_H

#include <cstddef>

#include "pawndb/types/client_conn.h"
#include "pawndb/types/ret_buf.h"

namespace PawnDB {

/**
 * @brief Database return message containing response data
 *
 * Owns a ret_buf (buffer + protocol composition) and tracks client connection.
 * Copy operations correctly manage buffer reference counting.
 */
class ret {
 public:
  /** @brief Initialize empty return message */
  constexpr ret() noexcept : buf_(), conn_() {}

  /** @brief Initialize return, moving buf_ref into internal ret_buf */
  ret(buf_ref&& _buffer, std::size_t _buffer_size, client_conn _conn) noexcept;

  // Copyable
  ret(const ret& other) noexcept;
  ret& operator=(const ret& other) noexcept;

  /** @brief Get mutable return buffer (composition + buffer access) */
  ret_buf& buf() noexcept;

  /** @brief Get const return buffer */
  const ret_buf& buf() const noexcept;

  /** @brief Get buffer size */
  std::size_t buf_sz() const noexcept;

  /** @brief Get client connection */
  const client_conn& conn() const noexcept;

 private:
  ret_buf buf_;      /**< Return buffer with protocol composition */
  client_conn conn_; /**< Client connection descriptor */
};

}  // namespace PawnDB

#endif
