#ifndef PAWNDB_TRAITS_COPY_H
#define PAWNDB_TRAITS_COPY_H

#include <type_traits>

namespace PawnDB {

template <typename Derived>
class CopyTrait {
 protected:
  // Protected constructor and destructor
  constexpr CopyTrait() {
    static_assert(std::is_copy_constructible<Derived>::value,
                  "CopyTrait requires copy constructible value type");
    static_assert(std::is_copy_assignable<Derived>::value,
                  "CopyTrait requires copy assignable value type");
  }

  ~CopyTrait() = default;
};

}  // namespace PawnDB

#endif  // PAWNDB_TRAITS_COPY_H
