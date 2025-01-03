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

#include "pawndb/data_types.h"
#include "pawndb/table.h"

namespace PawnDB {

using student_table = Table<16, student_name, std::uint8_t>;

template <typename T>
constexpr tp_id_t tbl_id() = delete;

template <>
constexpr tp_id_t tbl_id<student_table>() {
  return 1;
}

}  // namespace PawnDB

#endif  // PAWNDB_TABLE_TYPES_H
