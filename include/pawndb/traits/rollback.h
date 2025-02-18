#ifndef PAWNDB_TRAITS_ROLLBACK_H
#define PAWNDB_TRAITS_ROLLBACK_H

#include "pawndb/traits/parser.h"
#include "pawndb/types/buffer_table.h"
#include "pawndb/types/table_tuple_key.h"

namespace PawnDB {


/**
 * @brief A CRTP (Curiously Recurring Template Pattern) base class that provides
 * rollback-related traits for derived classes.
 *
 * @tparam Derived The derived class that inherits from this class template.
 * The derived class must implement the following methods:
 * - BufferRef trait_buf() const noexcept;
 * - TableTupleKey trait_key() const noexcept;
 * - OpType trait_op() const noexcept;
 */
template <typename Derived>
class RollbackTrait {
 public:
  /** @brief Get buffer reference */
  BufferRef buf() const noexcept {
    return derived().trait_buf();
  }

  /** @brief Get table-tuple key */
  TableTupleKey key() const noexcept {
    return derived().trait_key();
  }

  /** @brief Get operation type */
  OpType op() const noexcept {
    return derived().trait_op();
  }

 protected:
  // Protected constructor and destructor
  RollbackTrait() = default;
  ~RollbackTrait() = default;

  // CRTP helpers
  Derived& derived() noexcept {
    return static_cast<Derived&>(*this);
  }

  const Derived& derived() const noexcept {
    return static_cast<const Derived&>(*this);
  }
};

}  // namespace PawnDB

#endif  // PAWNDB_TRAITS_ROLLBACK_H
