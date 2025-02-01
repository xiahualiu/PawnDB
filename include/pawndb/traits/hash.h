#ifndef PAWNDB_TRAITS_HASH_H
#define PAWNDB_TRAITS_HASH_H

#include <cstddef>

namespace PawnDB {

/**
 * @brief CRTP base class for hash computation
 * @tparam Derived The derived class implementing hashing
 *
 * Required implementation:
 * - trait_hash() -> std::size_t
 */
template <typename Derived>
class HashTrait {
 public:
  /**
   * @brief Get hash value
   * @return Computed hash value
   */
  std::size_t hash() const noexcept {
    return derived().trait_hash();
  }

 protected:
  // Protected constructor and destructor
  HashTrait() = default;
  ~HashTrait() = default;

  // CRTP helpers
  Derived& derived() noexcept {
    return static_cast<Derived&>(*this);
  }

  const Derived& derived() const noexcept {
    return static_cast<const Derived&>(*this);
  }
};

}  // namespace PawnDB

#endif  // PAWNDB_TRAITS_HASH_H
