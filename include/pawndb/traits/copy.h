#ifndef PAWNDB_TRAITS_COPY_H
#define PAWNDB_TRAITS_COPY_H

/**
 * @file
 * @brief Copy trait for CRTP-based copyable objects.
 *
 * Alongside `MoveTrait`, PawnDB provides a small set of CRTP "trait"
 * classes to unify common operations across many types. `CopyTrait` offers a
 * uniform interface for copy semantics while leaving the actual copy logic to
 * the derived type.  It is intentionally minimal; users of the derived type
 * should implement the necessary operations themselves.
 *
 * Required implementation in the derived class:
 *  - `Derived trait_copy() const noexcept`           : produce a new
 *      instance that is a copy of `*this` (typically implemented by
 *      returning `Derived(*this)` or simply `return *this;`).
 *  - `void trait_copy_from(const Derived& other) noexcept` : copy state from
 *      `other` (usually `*this = other;`).
 *
 * The public API exposed by the trait mirrors the names used by
 * `MoveTrait`:
 *  * `Derived copy() const noexcept`          - returns a copied instance
 *  * `void copy_from(const Derived& other)`   - copy‑assign from `other`
 */

namespace PawnDB {

template <typename Derived>
class CopyTrait {
 public:
  /**
   * @brief type-level flag, true when a type supports the trait
   */
  constexpr static bool is_copyable = true;

  /**
   * @brief Create a new object by copying this one
   *
   * Calls the derived class's `trait_copy()` implementation. After the call
   * both objects contain equivalent state.
   */
  Derived copy() const noexcept {
    return static_cast<const Derived*>(this)->trait_copy();
  }

  /**
   * @brief Assign from another object using copy semantics
   *
   * Delegates to the derived class implementation of `trait_copy_from`.
   */
  void copy_from(const Derived& other) noexcept {
    static_cast<Derived*>(this)->trait_copy_from(other);
  }

 protected:
  // Protected ctor/dtor to prevent instantiation of the base class directly.
  CopyTrait() = default;
  ~CopyTrait() = default;
};

}  // namespace PawnDB

#endif  // PAWNDB_TRAITS_COPY_H
