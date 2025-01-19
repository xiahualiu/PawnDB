/**
 * @file table_types.h
 * @author Xiahua Liu @xiahualiu
 * @brief PawnDB table types.
 * @version 0.1
 * @date 2025-01-02
 *
 * @copyright MIT License
 *
 */

#ifndef PAWNDB_TABLE_TYPES_H
#define PAWNDB_TABLE_TYPES_H

#include "pawndb/types/student_table.h"

namespace PawnDB {

/**
 * @brief Template function to get the table ID for a given table type.
 *
 * This function is deleted for all types except the specializations provided
 * below.
 *
 * @tparam T The table type for which to get the table ID.
 * @return The table ID for the given table type.
 */
template <typename T>
constexpr tp_id_t tbl_id() = delete;

/**
 * @brief Specialization of tbl_id for student_table.
 *
 * @return The table ID for student_table.
 */
template <>
constexpr tp_id_t tbl_id<StudentTable>() {
  return 1;
}

}  // namespace PawnDB

#endif  // PAWNDB_TABLE_TYPES_H
