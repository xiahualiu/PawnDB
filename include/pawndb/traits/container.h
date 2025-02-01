#ifndef PAWNDB_TRAITS_CONTAINER_H
#define PAWNDB_TRAITS_CONTAINER_H

namespace PawnDB {

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

  /** @brief Clear container */
  void clear() noexcept {
    static_cast<Derived*>(this)->trait_clear();
  }

 protected:
  // Hide constructors
  ContainerTrait() = default;
  ~ContainerTrait() = default;

  // CRTP helpers
  Derived& derived() {
    return static_cast<Derived&>(*this);
  }
  const Derived& derived() const {
    return static_cast<const Derived&>(*this);
  }
};

}  // namespace PawnDB

#endif  // PAWNDB_TRAITS_CONTAINER_H
