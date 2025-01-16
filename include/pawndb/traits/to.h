#ifndef PAWNDB_TRAITS_TO_H
#define PAWNDB_TRAITS_TO_H

namespace PawnDB {

/**
 * @brief Type conversion trait
 * @tparam From Source type
 * @tparam To Target type
 */
template <typename Derived, typename To>
class ToTrait {
 public:
  /** @brief Convert to target type */
  To to() const noexcept {
    return static_cast<const Derived*>(this)->trait_to();
  }

 protected:
  // Protected constructor and destructor
  ToTrait() = default;
  ~ToTrait() = default;
};

}  // namespace PawnDB

#endif  // PAWNDB_TRAITS_TO_H