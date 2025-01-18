#ifndef PAWNDB_TRAITS_CONTAINER_H
#define PAWNDB_TRAITS_CONTAINER_H

namespace PawnDB {

/**
 * @brief CRTP interface for container implementations
 * @tparam Derived Class implementing container interface
 *
 * Provides common container operations like capacity checking and empty/full
 * status.
 *
 * Required trait implementations:
 * - trait_empty() -> bool
 * - trait_full() -> bool
 */
template <typename Derived>
class ContainerTrait {
 public:
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
  // Protected constructor and destructor
  ContainerTrait() = default;
  ~ContainerTrait() = default;
};

}  // namespace PawnDB

#endif  // PAWNDB_TRAITS_CONTAINER_H
