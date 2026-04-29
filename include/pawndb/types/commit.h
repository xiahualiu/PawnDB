#ifndef PAWNDB_TYPES_COMMIT_H
#define PAWNDB_TYPES_COMMIT_H

#include "pawndb/types/buf_ref.h"
#include "pawndb/types/job_buf.h"
#include "pawndb/types/unique_key.h"

namespace PawnDB {

/**
 * @brief Entry in commit table storing operation details
 *
 * Stores buffer reference, key, operation type and hash table flags.
 * Implements hash and copy operations for table storage.
 */
class commit {
 public:
  /** @brief Key type alias */
  using key_t = unique_key;

  /** @brief Default constructor creates invalid entry */
  constexpr commit() noexcept : buffer_(), key_(), op_(OpType::MAX_OP_VALUE) {}

  /**
   * @brief Construct entry with values
   * @param key Table-tuple key
   * @param op Operation type
   * @param buffer Associated buffer
   */
  commit(const unique_key& key, OpType op, const buf_ref& buffer) noexcept;

  // Copyable
  commit(const commit& other) noexcept;
  commit& operator=(const commit& other) noexcept;

  /** @brief Get buffer reference */
  buf_ref buf() const noexcept;

  /** @brief Get table-tuple key */
  const unique_key& key() const noexcept;

  /** @brief Get operation type */
  OpType op() const noexcept;

 private:
  buf_ref buffer_; /**< Associated buffer */
  unique_key key_; /**< Table-tuple key */
  OpType op_;      /**< Operation type */

  friend class commit_queue;
};

}  // namespace PawnDB

#endif  // PAWNDB_TYPES_COMMIT_H
