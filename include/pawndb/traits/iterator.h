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
    return static_cast<DerivedIter*>(this)->trait_next();
  }

  /** @brief Dereference operator
   *  @return Reference to current element */
  DerefType& operator*() noexcept {
    return static_cast<DerivedIter*>(this)->trait_deref();
  }

  /** @brief Pre-increment operator
   *  @return Reference to incremented iterator */
  DerivedIter& operator++() noexcept {
    return static_cast<DerivedIter*>(this)->trait_next();
  }

  /** @brief Post-increment operator
   *  @return Copy of iterator before increment */
  DerivedIter& operator++(int) noexcept {
    auto& tmp = *static_cast<DerivedIter*>(this);
    static_cast<DerivedIter*>(this)->trait_next();
    return tmp;
  }

 protected:
  // Protected constructor and destructor
  IterTypeTrait() = default;
  ~IterTypeTrait() = default;
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
    return static_cast<const Derived*>(this)->trait_begin();
  }

  DerivedIter end() const noexcept {
    return static_cast<const Derived*>(this)->trait_end();
  }

 protected:
  // Protected constructor and destructor
  IterTrait() = default;
  ~IterTrait() = default;
};

}  // namespace PawnDB

#endif
