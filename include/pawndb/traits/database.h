#ifndef PAWNDB_TRAITS_DATABASE_H
#define PAWNDB_TRAITS_DATABASE_H

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
  /** @brief Get the singleton instance of the database
   * @note Should only be used in main thread */
  static Derived& get_db_instance() {
    static Derived instance = Derived();
    return instance;
  }

 protected:
  // Protected constructor and destructor
  DatabaseTrait() = default;
  ~DatabaseTrait() = default;
};
}  // namespace PawnDB

#endif  // PAWNDB_TRAITS_DATABASE_H
