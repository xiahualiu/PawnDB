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

#include <atomic>

#include "pawndb/params.h"
#include "pawndb/types/buffer_table.h"
#include "pawndb/types/student_table.h"

namespace PawnDB {

/**
 * @brief Class representing the demo database.
 */
class Database {
 public:
  std::uint32_t get_current_tickstamp_() const noexcept {
    return tickstamp_.load(std::memory_order_relaxed);
  }

  void increment_tickstamp_() noexcept {
    tickstamp_.fetch_add(1, std::memory_order_relaxed);
  }

  void clear_() noexcept {
    tickstamp_.store(0, std::memory_order_relaxed);
    students_.clear_();
    buffers_.clear_();
  }

  StudentTable students_;
  std::atomic_uint32_t tickstamp_;

  alignas(BUFFER_ALIGNMENT) BufferTable buffers_;
};

}  // namespace PawnDB

#endif  // PAWNDB_SCHEMA_DEMO_H
