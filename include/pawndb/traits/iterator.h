#ifndef PAWNDB_TRAITS_ITERATOR_H
#define PAWNDB_TRAITS_ITERATOR_H

namespace PawnDB {

/**
 * @brief CRTP base class for iterator implementation
 * @tparam DerivedIterator The derived iterator class
 * @tparam DerefType Type of elements being iterated
 *
 * Required implementations:
 * - trait_next()
 * - trait_deref()
 * - trait_equals(const DerivedIterator&)
 */
template <typename DerivedIterator, typename DerefType>
class IteratorTypeTrait {
 public:
  /** @brief Advance iterator to next element */
  void next() noexcept {
    return static_cast<DerivedIterator*>(this)->trait_next();
  }

  /** @brief Dereference operator
   *  @return Reference to current element */
  DerefType& operator*() noexcept {
    return static_cast<DerivedIterator*>(this)->trait_deref();
  }

  /** @brief Equality comparison
   *  @param other Iterator to compare with
   *  @return true if iterators are equal */
  bool operator==(const DerivedIterator& other) const noexcept {
    return static_cast<const DerivedIterator*>(this)->trait_equals(other);
  }

  /** @brief Inequality comparison
   *  @param other Iterator to compare with
   *  @return true if iterators are not equal */
  bool operator!=(const DerivedIterator& other) const noexcept {
    return !static_cast<const DerivedIterator*>(this)->trait_equals(other);
  }

  /** @brief Pre-increment operator
   *  @return Reference to incremented iterator */
  DerivedIterator& operator++() noexcept {
    return static_cast<DerivedIterator*>(this)->trait_next();
  }

  /** @brief Post-increment operator
   *  @return Copy of iterator before increment */
  DerivedIterator operator++(int) noexcept {
    auto tmp = *this;
    static_cast<DerivedIterator*>(this)->trait_next();
    return tmp;
  }
};

/**
 * @brief CRTP base class for iterable containers
 * @tparam Derived The derived container class
 * @tparam DerivedIterator The iterator type
 *
 * Required implementations:
 * - trait_begin()
 * - trait_end()
 */
template <typename Derived, typename DerivedIterator>
class IteratorTrait {
 protected:
  IteratorTrait() = default;
  ~IteratorTrait() = default;

 public:
  DerivedIterator begin() noexcept {
    return static_cast<const Derived*>(this)->trait_begin();
  }

  DerivedIterator end() noexcept {
    return static_cast<const Derived*>(this)->trait_end();
  }
};

}  // namespace PawnDB

#endif
