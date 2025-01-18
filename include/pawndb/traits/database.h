#ifndef PAWNDB_TRAITS_DATABASE_H
#define PAWNDB_TRAITS_DATABASE_H

#include "pawndb/params.h"

namespace PawnDB {

/**
 * @brief CRTP base class for database management
 * @tparam Derived The derived database class
 *
 * Required trait implementations:
 * - trait_get_current_tickstamp() -> std::uint32_t
 * - trait_increment_tickstamp() -> void
 */
template <typename Derived>
class DatabaseTrait {
 public:
  /** @brief Get current database tick timestamp
   * @return Current tick timestamp value */
  tick_t get_current_tickstamp() const noexcept {
    return static_cast<const Derived*>(this)->trait_get_current_tickstamp();
  }

  /** @brief Increment database tick timestamp */
  void increment_tickstamp() noexcept {
    static_cast<Derived*>(this)->trait_increment_tickstamp();
  }

  /** @brief Clear the database of all tables */
  void clear() noexcept {
    static_cast<Derived*>(this)->trait_clear();
  }

  /** @brief Get the singleton instance of the database
   * @note Should be used with caution */
  static Derived& get_db_instance() {
    static Derived instance = Derived();
    return instance;
  }

  /** @brief Clear the singleton instance of the database
   * @note Should be used with caution */
  static void clear_db_instance() {
    static Derived& instance = get_db_instance();
    instance.trait_clear();
  }

 protected:
  // Protected constructor and destructor
  DatabaseTrait() = default;
  ~DatabaseTrait() = default;
};
}  // namespace PawnDB

#endif
