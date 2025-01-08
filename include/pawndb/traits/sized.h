/**
 * @brief CRTP interface for size-aware containers
 * @tparam Derived Class implementing the size interface
 * @tparam SizeType Type used for size values
 *
 * Required implementations:
 * - SizeType trait_size() const
 * - SizeType trait_capacity() const
 * - bool trait_empty() const
 * - bool trait_full() const
 */
#ifndef PAWNDB_TRAITS_SIZED_H
#define PAWNDB_TRAITS_SIZED_H

#include <cstddef>

namespace PawnDB {

/**
 * @brief CRTP interface for classes that provide size information
 * @tparam Derived Class implementing the size interface
 *
 * Any class inheriting from Sized must implement:
 * - size_t user_size() const noexcept
 */
template <typename Derived>
class Sized {
 public:
  /**
   * @brief Get current size
   * @return Number of elements
   */
  std::size_t size() const noexcept {
    return static_cast<const Derived*>(this)->trait_size();
  }

 protected:
  Sized() = default;
  ~Sized() = default;

};

}  // namespace PawnDB

#endif  // PAWNDB_TRAITS_SIZED_H
