#ifndef PAWNDB_TRAITS_CONTAINER_H
#define PAWNDB_TRAITS_CONTAINER_H

#include <cstddef>

namespace PawnDB {

/**
 * @brief CRTP interface for container implementations
 * @tparam Derived Class implementing container interface
 */
template <typename Derived>
class Container {
 public:
  /**
   * @brief Get container capacity
   * @return Maximum number of elements
   */
  constexpr std::size_t capacity() const noexcept {
    return static_cast<const Derived*>(this)->trait_capacity();
  }

  /**
   * @brief Check if container is empty
   * @return true if no elements present
   */
  bool empty() const noexcept {
    return static_cast<const Derived*>(this)->trait_empty();
  }

  /**
   * @brief Check if container is full
   * @return true if at capacity
   */
  bool full() const noexcept {
    return static_cast<const Derived*>(this)->trait_full();
  }

 protected:
  Container() = default;
  ~Container() = default;
};

}  // namespace PawnDB

#endif  // PAWNDB_TRAITS_CONTAINER_H
