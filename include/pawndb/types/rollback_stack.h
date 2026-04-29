#ifndef PAWNDB_TYPES_ROLLBACK_STACK_H
#define PAWNDB_TYPES_ROLLBACK_STACK_H

#include <array>
#include <cstddef>

#include "pawndb/params.h"
#include "pawndb/result.h"
#include "pawndb/types/rollback.h"

namespace PawnDB {

/**
 * @brief Fixed-size LIFO stack for storing rollback entries
 *
 * Features:
 * - O(1) push and pop
 * - LIFO ordering for reverse-order undo during abort
 * - Fixed maximum size per transaction
 */
class rollback_stack {
 private:
  /** @brief Maximum entries per transaction */
  static constexpr std::size_t N = MAX_COMMIT_PER_TRANSACTION;

 public:
  /** @brief Result type for stack operations */
  using stack_r = Result<rollback, QueueError>;

  /** @brief Default constructor initializes empty stack */
  constexpr rollback_stack() noexcept : rollbacks_(), size_(0) {}

  // Not copyable
  rollback_stack(const rollback_stack& other) noexcept = delete;
  rollback_stack& operator=(const rollback_stack& other) noexcept = delete;

  /** @brief Push rollback entry onto stack */
  QueueError push(const rollback& entry) noexcept;

  /** @brief Peek at top entry without removing */
  stack_r top() noexcept;

  /** @brief Remove top entry */
  void pop() noexcept;

  /** @brief Clear all entries */
  void clear() noexcept;

  /** @brief Get number of entries */
  std::size_t size() const noexcept;

  /** @brief Check if stack is empty */
  bool empty() const noexcept;

  /** @brief Check if stack is full */
  bool full() const noexcept;

 private:
  std::array<rollback, N> rollbacks_; /**< Entry storage */
  std::size_t size_;                  /**< Number of entries */
};

}  // namespace PawnDB

#endif  // PAWNDB_TYPES_ROLLBACK_STACK_H
