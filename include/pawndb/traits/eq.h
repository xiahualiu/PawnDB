#ifndef PAWNDB_TRAITS_EQ_H
#define PAWNDB_TRAITS_EQ_H

namespace PawnDB {

/**
 * @brief CRTP base class for equality comparison
 * @tparam Derived The derived class implementing equality
 *
 * Required implementation:
 * - trait_equals(const Derived&) -> bool
 */
template <typename Derived, typename T>
class EqTrait {
 public:
  /**
   * @brief Equality comparison operator
   * @param _other Object to compare with
   * @return true if objects are equal
   */
  bool operator==(const T& _other) const noexcept {
    return derived().trait_equals(_other);
  }

  /**
   * @brief Inequality comparison operator
   * @param _other Object to compare with
   * @return true if objects are not equal
   */
  bool operator!=(const T& _other) const noexcept {
    return !derived().trait_equals(_other);
  }

 protected:
  // Protected constructor and destructor
  EqTrait() = default;
  ~EqTrait() = default;

  // CRTP helpers
  Derived& derived() {
    return static_cast<Derived&>(*this);
  }

  const Derived& derived() const {
    return static_cast<const Derived&>(*this);
  }
};

}  // namespace PawnDB

#endif  // PAWNDB_TRAITS_EQ_H
