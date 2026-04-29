#ifndef PAWNDB_TYPES_LOCK_H
#define PAWNDB_TYPES_LOCK_H

#include "pawndb/params.h"
#include "pawndb/types/unique_key.h"

namespace PawnDB {

/** @brief Lock operation error codes */
enum class LockError {
  None,     /**< Operation successful */
  Full,     /**< Lock table full */
  NotFound, /**< Lock not found */
  Conflict  /**< Lock already held */
};

/**
 * @brief Lock entry storing lock information
 */
class lock_entry {
 public:
  using key_t = unique_key; /**< Key type alias */

  /** @brief Default constructor creates invalid entry */
  constexpr lock_entry() noexcept
      : key_(),
        type_(LockType::EXCLUSIVE),
        is_used_(false),
        is_deleted_(false) {}

  /** @brief Construct lock entry with values
   *  @param _key Tuple key
   *  @param _type Lock type */
  lock_entry(const unique_key& _key, LockType _type) noexcept;

  // Copyable
  lock_entry(const lock_entry& other) noexcept;
  lock_entry& operator=(const lock_entry& other) noexcept;

  /** @brief Get lock type */
  LockType lock_type() const noexcept;

  /** @brief Get lock key */
  const unique_key& key() const noexcept;

 private:
  unique_key key_;  /**< Tuple identifier */
  LockType type_;   /**< Lock mode */
  bool is_used_;    /**< Usage flag */
  bool is_deleted_; /**< Deletion flag */

  friend class lock_list;
  friend class LockRecordIterator;
};

}  // namespace PawnDB

#endif  // PAWNDB_TYPES_LOCK_H
