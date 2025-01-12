#ifndef PAWNDB_TRAITS_TABLE_H
#define PAWNDB_TRAITS_TABLE_H

#include "pawndb/result.h"


namespace PawnDB {

/**
 * @brief Table operation error codes
 */
enum class TableError {
  None,     /**< Operation successful */
  NotFound, /**< Key not found */
  Full,     /**< Table at capacity */
  Conflict  /**< Key already exists */
};

/**
 * @brief CRTP interface for hash table implementations
 * @tparam Derived Class implementing table interface
 * @tparam EntryType Type of table entries, must implement HashTrait
 *
 * Required trait implementations:
 * - trait_insert(const entry_type&) -> TableR
 * - trait_search(const key_type&) -> TableR
 * - trait_remove(const key_type&) -> TableError
 * - trait_write(const entry_type&) -> TableR
 */
template <typename Derived, typename EntryType>
class HashTableTrait {
 public:
  /** @brief Entry type alias */
  using entry_type = EntryType;

  /** @brief Key type alias */
  using key_t = typename EntryType::key_t;

  /** @brief Table operation result type */
  using table_r = Result<EntryType&, TableError>;

  /**
   * @brief Insert new entry into table
   * @param _entry Entry to insert
   * @return Result containing reference to inserted entry or error
   */
  table_r insert(const entry_type& _entry) noexcept {
    return static_cast<Derived*>(this)->trait_insert(_entry);
  }

  /**
   * @brief Search for entry by key
   * @param _key Key to search for
   * @return Result containing reference to found entry or error
   */
  table_r search(const key_t& _key) noexcept {
    return static_cast<Derived*>(this)->trait_search(_key);
  }

  /**
   * @brief Remove entry by key
   * @param _key Key of entry to remove
   * @return Error status of operation
   */
  TableError remove(const key_t& _key) noexcept {
    return static_cast<Derived*>(this)->trait_remove(_key);
  }

  /**
   * @brief Update existing entry
   * @param _entry Entry with updated values
   * @return Result containing reference to updated entry or error
   */
  table_r write(const entry_type& _entry) noexcept {
    return static_cast<Derived*>(this)->trait_write(_entry);
  }

 protected:
  HashTableTrait() = default;
  ~HashTableTrait() = default;
};

}  // namespace PawnDB

#endif  // PAWNDB_TRAITS_TABLE_H
