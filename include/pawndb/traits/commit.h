#ifndef PAWNDB_TRAITS_COMMIT_H
#define PAWNDB_TRAITS_COMMIT_H

#include "pawndb/traits/parser.h"
#include "pawndb/types/buffer_table.h"
#include "pawndb/types/table_tuple_key.h"

namespace PawnDB {

/**
 * @brief CRTP interface for commit entry implementations
 * @tparam Derived The derived commit class
 */
template <typename Derived>
class CommitTrait {
 public:
  /** @brief Get buffer reference */
  BufferRef buffer() const noexcept {
    return static_cast<const Derived*>(this)->trait_buffer();
  }

  /** @brief Get table-tuple key */
  const TableTupleKey& key() const noexcept {
    return static_cast<const Derived*>(this)->trait_key();
  }

  /** @brief Get operation type */
  OpType op() const noexcept {
    return static_cast<const Derived*>(this)->trait_op();
  }

 protected:
  // Protected constructor and destructor
  CommitTrait() = default;
  ~CommitTrait() = default;
};

}  // namespace PawnDB

#endif  // PAWNDB_TRAITS_COMMIT_H
