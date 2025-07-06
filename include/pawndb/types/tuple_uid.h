#ifndef PAWNDB_TYPES_TUPLE_UID_H
#define PAWNDB_TYPES_TUPLE_UID_H

#include "pawndb/params.h"

namespace PawnDB {

/**
 * @brief Unique identifier for database tuples
 *
 * Represents a globally unique identifier for tuples within the database.
 * Combines table ID, tuple ID, and transaction ID to ensure uniqueness
 * across tables and transactions.
 */
class TupleUID {
 public:
  /** @brief Default constructor - creates invalid UID */
  constexpr TupleUID() noexcept : table_id_(0), tuple_id_(0) {}

  /**
   * @brief Construct UID from components
   * @param table_id Table identifier
   * @param tuple_id Tuple identifier within the table
   */
  constexpr TupleUID(tp_id_t table_id, tp_id_t tuple_id) noexcept
      : table_id_(table_id), tuple_id_(tuple_id) {}

  // Copyable
  TupleUID(const TupleUID& other) noexcept = default;
  TupleUID& operator=(const TupleUID& other) noexcept = default;

  // Movable
  TupleUID(TupleUID&& other) noexcept = default;
  TupleUID& operator=(TupleUID&& other) noexcept = default;

 private:
  tp_id_t table_id_;  ///< Table identifier
  tp_id_t tuple_id_;  ///< Tuple identifier within table
};

}  // namespace PawnDB

#endif  // PAWNDB_TYPES_TUPLE_UID_H
