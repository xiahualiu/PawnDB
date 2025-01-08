/**
 * @file tuple_key.h
 * @brief CRTP interface for tuple key management
 * @version 0.1
 * @date 2025-01-02
 *
 * Features:
 * - Key generation
 * - Hash computation
 * - Equality comparison
 * - Sequential key values
 */
#ifndef PAWNDB_TRAITS_TUPLE_KEY_H
#define PAWNDB_TRAITS_TUPLE_KEY_H

#include <cstddef>

namespace PawnDB {

/**
 * @brief CRTP interface for tuple key implementations
 * @tparam Derived Class implementing key interface
 *
 * Required implementations:
 * - Derived trait_next() const
 * - std::size_t trait_hash() const
 * - bool trait_equals(const Derived&) const
 */
template <typename Derived>
class TupleKeyTrait {
 public:
 
     /**
     * @brief Generate next sequential key
     * @return Next key value
     */
  Derived next() const noexcept {
    return static_cast<const Derived*>(this)->trait_next();
  }

    /**
     * @brief Compute hash value for key
     * @return Hash value
     */
  std::size_t hash() const noexcept {
    return static_cast<const Derived*>(this)->trait_hash();
  }

    /**
     * @brief Compare keys for equality
     * @param other Key to compare with
     * @return true if keys equal
     */
  bool equals(const Derived& other) const noexcept {
    return static_cast<const Derived*>(this)->trait_equals(other);
  }

  bool operator==(const Derived& other) const noexcept { return equals(other); }


  bool operator!=(const Derived& other) const noexcept {
    return !equals(other);
  }

 protected:
  TupleKeyTrait() = default;
  ~TupleKeyTrait() = default;
};

}  // namespace PawnDB

#endif  // PAWNDB_TRAITS_KEY_H
