#ifndef PAWNDB_TRAITS_MOVE_H
#define PAWNDB_TRAITS_MOVE_H

/**
 * @file
 * @brief Move trait for CRTP-based movable objects.
 *
 * PawnDB uses a collection of small CRTP "trait" classes to unify common
 * operations across many types.  `MoveTrait` is the complement of
 * `CopyTrait` and provides a simple interface for performing move operations
 * in a uniform way.  It is intentionally very lightweight; the derived class
 * is responsible for implementing the actual move logic.
 *
 * Required implementation in the derived class:
 *  - `Derived trait_move() noexcept`                  : create a new object by
 *      moving from `*this` (typically implemented via `return
 * std::move(*this);`)
 *  - `void trait_move_from(Derived&& other) noexcept`: assign from a
 *      rvalue reference (usually just `*this = std::move(other);`).
 *
 * The public API exposed by the trait mirrors the names used by
 * `CopyTrait`:
 *  * `Derived move() noexcept`            - returns a moved instance
 *  * `void move_from(Derived&& other)`    - move‑assign from `other`
 *
 *
 */

#include <utility>  // for std::move (not strictly required by the header but
                    // convenient for inline implementations in derived types)

namespace PawnDB {

template <typename Derived>
class MoveTrait {
 public:
  /**
   * @brief type-level flag, true when a type supports the trait
   */
  constexpr static bool is_movable = true;

  /**
   * @brief Extract a value by moving from this object
   *
   * Calls the derived class implementation of `trait_move()`.
   * After the call the current object will be in a valid, but unspecified,
   * state (typically the moved‑from state produced by the move
   * constructor/assignment of the derived type).
   */
  Derived move() noexcept {
    return static_cast<Derived*>(this)->trait_move();
  }

  /**
   * @brief Assign from another object using move semantics
   *
   * Delegates to the derived class implementation of
   * `trait_move_from`.
   */
  void move_from(Derived&& other) noexcept {
    static_cast<Derived*>(this)->trait_move_from(std::move(other));
  }

 protected:
  // Protected ctor/dtor to prevent instantiation of the base class directly.
  MoveTrait() = default;
  ~MoveTrait() = default;
};

}  // namespace PawnDB

#endif  // PAWNDB_TRAITS_MOVE_H
