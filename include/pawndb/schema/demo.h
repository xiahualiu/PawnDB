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

#include "pawndb/params.h"
#include "pawndb/types/buffer_table.h"
#include "pawndb/types/student_table.h"

namespace PawnDB {

/**
 * @brief Class representing the demo database.
 */
class Database {
 public:
  /**
   * @brief Constructs a new Database object.
   */
  Database() = default;

  StudentTable students;

  alignas(BUFFER_ALIGNMENT) BufferTable buffers;
};

}  // namespace PawnDB

#endif  // PAWNDB_SCHEMA_DEMO_H
