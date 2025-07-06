#ifndef PAWNDB_TYPES_COMMIT_TABLE_H
#define PAWNDB_TYPES_COMMIT_TABLE_H

#include <array>
#include <cstddef>

#include "pawndb/params.h"
#include "pawndb/types/parser.h"
#include "pawndb/types/buffer_table.h"
#include "pawndb/types/tuple_uid.h"

namespace PawnDB {

/**
 * @brief Entry in commit table storing operation details
 *
 * Stores buffer reference, key, operation type and hash table flags.
 * Implements hash and copy operations for table storage.
 */
class RollbackEntry {
 public:
  /** @brief Key type alias */
  using key_t = TupleUID;

  /** @brief Default constructor creates invalid entry */
  constexpr RollbackEntry() noexcept
      : buffer_(), key_(), op_(OpType::MAX_OP_VALUE) {}

};

/**
 * @brief Fixed-size hash table for storing transaction commit entries
 *
 * Features:
 * - O(1) lookup and insertion
 * - Iterator support for traversing entries
 * - Fixed maximum size per transaction
 *
 * Implemented traits:
 * - TableTrait: Hash table operations
 * - IterTrait: Iterator support
 * - Sized: Size tracking
 * - Container: Capacity operations
 */
class RollbackDeque : public DequeTrait<RollbackDeque, RollbackEntry>,
                      public SizedTrait<RollbackDeque>,
                      public ContainerTrait<RollbackDeque> {
  /** @brief Maximum entries per transaction */
  static constexpr std::uint8_t N = MAX_COMMIT_PER_TRANSACTION;

 public:
  /** @brief Default constructor initializes empty table */
  constexpr RollbackDeque() noexcept
      : rollbacks_(), head_(0), tail_(0), size_(0) {}

  // Not copyable
  RollbackDeque(const RollbackDeque& other) noexcept = delete;
  RollbackDeque& operator=(const RollbackDeque& other) noexcept = delete;

  // Deque trait
  /** @brief Push entry to the back of the deque
   * @param entry Entry to push
   * @return Error code */
  DequeError trait_push_back(const RollbackEntry& entry) noexcept;

  /** @brief Get front entry */
  deque_r trait_front() noexcept;

  /** @brief Get back entry */
  deque_r trait_back() noexcept;

  /** @brief Pop front entry
    * @return Error code */
  DequeError trait_pop_front() noexcept;

  /** @brief Pop back entry
    * @return Error code */
  DequeError trait_pop_back() noexcept;

  // Size trait
  /** @brief Get current entry count */
  std::size_t trait_size() const noexcept;

  // Container trait
  /** @brief Check if deque is full */
  bool trait_full() const noexcept;

  /** @brief Check if deque is empty */
  bool trait_empty() const noexcept;

  /** @brief Clear all entries */
  void trait_clear() noexcept;

 private:
  std::array<RollbackEntry, N> rollbacks_; /**< Entry storage */
  std::uint8_t head_;                      /**< Read position */
  std::uint8_t tail_;                      /**< Write position */
  std::uint8_t size_;                      /**< Number of entries */
};

}  // namespace PawnDB

#endif  // PAWNDB_TYPES_COMMIT_TABLE_H
