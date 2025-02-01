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
 * @brief CRTP interface for table implementations
 * @tparam Derived Class implementing table interface
 * @tparam EntryType Type of table entries, must implement HashTrait
 *
 * Required trait implementations:
 * - trait_insert(const entry_type&) -> table_r
 * - trait_search(const key_type&) -> table_r
 * - trait_remove(const key_type&) -> TableError
 * - trait_update(const entry_type&) -> TableError
 */
template <typename Derived, typename EntryType>
class TableTrait {
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
    return derived().trait_insert(_entry);
  }

  /**
   * @brief Search for entry by key
   * @param _key Key to search for
   * @return Result containing reference to found entry or error
   */
  table_r search(const key_t& _key) noexcept {
    return derived().trait_search(_key);
  }

  /**
   * @brief Remove entry by key
   * @param _key Key of entry to remove
   * @return Error status of operation
   */
  TableError remove(const key_t& _key) noexcept {
    return derived().trait_remove(_key);
  }

 protected:
  // Protected constructor and destructor
  TableTrait() = default;
  ~TableTrait() = default;

  // CRTP helpers
  Derived& derived() noexcept {
    return static_cast<Derived&>(*this);
  }

  const Derived& derived() const noexcept {
    return static_cast<const Derived&>(*this);
  }
};

}  // namespace PawnDB

#endif  // PAWNDB_TRAITS_TABLE_H
