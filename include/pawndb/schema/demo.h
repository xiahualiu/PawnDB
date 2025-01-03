/**
 * @file demo.h
 * @author Xiahua Liu @xiahualiu
 * @brief PawnDB demo schema.
 * @version 0.1
 * @date 2025-01-02
 *
 * @copyright MIT License
 *
 */

#ifndef PAWNDB_SCHEMA_DEMO_H
#define PAWNDB_SCHEMA_DEMO_H

#include <tuple>

#include "pawndb/params.h"
#include "pawndb/buffer_table.h"
#include "pawndb/table_types.h"

namespace PawnDB {

class Database {
 public:
  Database() = default;

  std::tuple<student_table> table;
  alignas(BUFFER_ALIGNMENT) BufferTable buffers;
};

}  // namespace PawnDB

#endif  // PAWNDB_SCHEMA_DEMO_H
