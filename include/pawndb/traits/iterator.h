#ifndef PAWNDB_TRAITS_ITERATOR_H
#define PAWNDB_TRAITS_ITERATOR_H

namespace PawnDB {

/**
 * @brief CRTP base class for iterator implementation
 * @tparam DerivedIter The derived iterator class
 * @tparam DerefType Type of elements being iterated
 * @note It can be a const iterator by passing a const type to DerefType
 *
 * Required implementations:
 * - trait_next()
 * - trait_deref()
 */
template <typename DerivedIter, typename DerefType>
class IterTypeTrait {
 public:
  /** @brief Advance iterator to next element */
  void next() noexcept {
    return derived().trait_next();
  }

  /** @brief Dereference operator
   *  @return Reference to current element */
  DerefType& operator*() noexcept {
    return derived().trait_deref();
  }

  /** @brief Pre-increment operator
   *  @return Reference to incremented iterator */
  DerivedIter& operator++() noexcept {
    return derived().trait_next();
  }

  /** @brief Post-increment operator
   *  @return Copy of iterator before increment */
  DerivedIter& operator++(int) noexcept {
    auto& tmp = derived();
    derived().trait_next();
    return tmp;
  }

 protected:
  // Protected constructor and destructor
  IterTypeTrait() = default;
  ~IterTypeTrait() = default;

  // CRTP helpers
  DerivedIter& derived() noexcept {
    return static_cast<DerivedIter&>(*this);
  }

  const DerivedIter& derived() const noexcept {
    return static_cast<const DerivedIter&>(*this);
  }
};

/**
 * @brief CRTP base class for iterable containers
 * @tparam Derived The derived container class
 * @tparam DerivedIter The iterator type
 *
 * Required implementations:
 * - trait_begin()
 * - trait_end()
 */
template <typename Derived, typename DerivedIter>
class IterTrait {
 public:
  DerivedIter begin() const noexcept {
    return derived().trait_begin();
  }

  DerivedIter end() const noexcept {
    return derived().trait_end();
  }

 protected:
  // Protected constructor and destructor
  IterTrait() = default;
  ~IterTrait() = default;

  // CRTP helpers
  Derived& derived() noexcept {
    return static_cast<Derived&>(*this);
  }

  const Derived& derived() const noexcept {
    return static_cast<const Derived&>(*this);
  }
};

}  // namespace PawnDB

#endif
