#ifndef PAWNDB_SCHEMA_DEMO_H
#define PAWNDB_SCHEMA_DEMO_H

#include <atomic>

#include "pawndb/params.h"
#include "pawndb/traits/database.h"
#include "pawndb/types/buffer_manager.h"
#include "pawndb/types/student_table.h"

namespace PawnDB {

/**
 * @brief Class representing the demo database.
 */
class Database : public DatabaseTrait<Database> {
  std::uint32_t trait_get_current_tickstamp() const noexcept {
    return tickstamp_.load(std::memory_order_relaxed);
  }

  void trait_increment_tickstamp() noexcept {
    tickstamp_.fetch_add(1, std::memory_order_relaxed);
  }

  void trait_clear() noexcept {
    tickstamp_.store(0, std::memory_order_relaxed);
    students_.clear();
    buffers_.clear();
  }

 public:
  StudentTable students_;
  std::atomic_uint32_t tickstamp_;

  alignas(BUFFER_ALIGNMENT) BufferTable buffers_;
};

}  // namespace PawnDB

#endif  // PAWNDB_SCHEMA_DEMO_H
