#ifndef PAWNDB_SCHEMA_DEMO_H
#define PAWNDB_SCHEMA_DEMO_H

#include "pawndb/params.h"
#include "pawndb/traits/database.h"
#include "pawndb/types/buffer_table.h"
#include "pawndb/types/student_table.h"

namespace PawnDB {

/** @brief Class representing the demo database. */
class Database : public DatabaseTrait<Database> {
 public:
  StudentTable students_;
  alignas(BUFFER_ALIGNMENT) BufferTable buffers_;
};

}  // namespace PawnDB

#endif  // PAWNDB_SCHEMA_DEMO_H
