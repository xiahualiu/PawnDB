#ifndef PAWNDB_TRAITS_COPY_H
#define PAWNDB_TRAITS_COPY_H

/**
 * @brief Trait class providing copy functionality through CRTP pattern
 *
 * This trait provides copy semantics to derived classes through the Curiously
 * Recurring Template Pattern (CRTP). Classes inheriting from this trait must
 * implement:
 * - trait_clone(): Creates a copy of the derived object
 * - trait_copy(const Derived&): Copies state from another derived object
 *
 * @note The copy can be lazy, which means it does not need to be exactly 1:1
 * memory copy. The requirement is simple, you need to make sure the object's
 * other functions behaviors should be the same after copy.
 */
namespace PawnDB {

template <typename Derived>
class CopyTrait {
 public:
  constexpr static bool is_copyable = true;

  /** @brief Create a copy of this object */
  Derived clone() const noexcept {
    return static_cast<const Derived*>(this)->trait_clone();
  }

  /** @brief Copy from another object */
  void copy(const Derived& other) noexcept {
    static_cast<Derived*>(this)->trait_copy(other);
  }

 protected:
  CopyTrait() = default;
  ~CopyTrait() = default;
};

}  // namespace PawnDB

#endif  // PAWNDB_TRAITS_COPY_H
