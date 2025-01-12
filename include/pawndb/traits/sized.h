#ifndef PAWNDB_TRAITS_SIZED_H
#define PAWNDB_TRAITS_SIZED_H

#include <cstddef>

namespace PawnDB {

/**
 * @brief CRTP interface for classes that provide size information
 * @tparam Derived Class implementing the size interface
 *
 * Required trait implementation:
 * - trait_size() -> std::size_t : Current element count
 */
template <typename Derived>
class SizedTrait {
 public:
  /**
   * @brief Get current size
   * @return Number of elements
   */
  std::size_t size() const noexcept {
    return static_cast<const Derived*>(this)->trait_size();
  }

 protected:
  SizedTrait() = default;
  ~SizedTrait() = default;
};

}  // namespace PawnDB

#endif  // PAWNDB_TRAITS_SIZED_H
