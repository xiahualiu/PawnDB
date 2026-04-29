#ifndef PAWNDB_TYPES_ROLLBACK_H
#define PAWNDB_TYPES_ROLLBACK_H

#include "pawndb/types/buf_ref.h"
#include "pawndb/types/job_buf.h"
#include "pawndb/types/unique_key.h"

namespace PawnDB {

/**
 * @brief Entry in rollback stack storing undo information
 *
 * Stores buffer reference, key, and operation type needed to undo
 * a committed operation during transaction abort.
 */
class rollback {
 public:
  /** @brief Key type alias */
  using key_t = unique_key;

  /** @brief Default constructor creates invalid entry */
  constexpr rollback() noexcept
      : buffer_(), key_(), op_(OpType::MAX_OP_VALUE) {}

  /**
   * @brief Construct entry with values
   * @param key Table-tuple key
   * @param op Original operation type
   * @param buffer Before-image buffer
   */
  rollback(const unique_key& key, OpType op, const buf_ref& buffer) noexcept;

  // Copyable
  rollback(const rollback& other) noexcept;
  rollback& operator=(const rollback& other) noexcept;

  /** @brief Get buffer reference */
  buf_ref buf() const noexcept;

  /** @brief Get table-tuple key */
  const unique_key& key() const noexcept;

  /** @brief Get original operation type */
  OpType op() const noexcept;

 private:
  buf_ref buffer_; /**< Before-image buffer */
  unique_key key_; /**< Table-tuple key */
  OpType op_;      /**< Original operation type */

  friend class rollback_stack;
};

}  // namespace PawnDB

#endif  // PAWNDB_TYPES_ROLLBACK_H
