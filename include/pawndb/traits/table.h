#ifndef PAWNDB_TRAITS_TABLE_H
#define PAWNDB_TRAITS_TABLE_H

#include "pawndb/result.h"

namespace PawnDB {

enum class TableError {
  None,     /**< Operation successful */
  NotFound, /**< Key not found */
  Full,     /**< Table at capacity */
  Conflict  /**< Key already exists */
};

/**
 * @brief CRTP interface for table implementations
 * @tparam Derived Class implementing table interface
 */
template <typename Derived, typename EntryType>
class TableTrait {
 public:
  using entry_type = EntryType;
  using key_type = typename EntryType::key_type;

  using TableR = Result<EntryType&, TableError>;

  TableR insert(const entry_type& _entry) noexcept {
    return static_cast<Derived*>(this)->trait_insert(_entry);
  }

  TableR search(const key_type& _key) noexcept {
    return static_cast<Derived*>(this)->trait_search(_key);
  }

  TableError remove(const key_type& _key) noexcept {
    return static_cast<Derived*>(this)->trait_remove(_key);
  }

  TableR write(const entry_type& _entry) noexcept {
    return static_cast<Derived*>(this)->trait_write(_entry);
  }

 protected:
  TableTrait() = default;
  ~TableTrait() = default;
};

}  // namespace PawnDB

#endif  // PAWNDB_TRAITS_TABLE_H
