#ifndef PAWNDB_TRAITS_DATABASE_H
#define PAWNDB_TRAITS_DATABASE_H

#include <cstdint>

namespace PawnDB {

template <typename Derived>
class DatabaseTrait {
 public:
  std::uint32_t get_current_tickstamp() const noexcept {
    return static_cast<const Derived*>(this)->trait_get_current_tickstamp();
  }

  void increment_tickstamp() noexcept {
    static_cast<Derived*>(this)->trait_increment_tickstamp();
  }

 private:
  DatabaseTrait() = delete;
  ~DatabaseTrait() = delete;
};
}  // namespace PawnDB

#endif
