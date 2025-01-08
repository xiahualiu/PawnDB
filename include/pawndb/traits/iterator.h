#ifndef PAWNDB_TRAITS_ITERATOR_H
#define PAWNDB_TRAITS_ITERATOR_H

namespace PawnDB {

template <typename DerivedIterator, typename T>
class IteratorTypeTrait {
 public:
  void next() noexcept {
    return static_cast<DerivedIterator*>(this)->trait_next();
  }

  T& operator*() noexcept {
    return static_cast<DerivedIterator*>(this)->trait_deref();
  }

  bool operator==(const DerivedIterator& other) const noexcept {
    return static_cast<const DerivedIterator*>(this)->trait_equals(other);
  }

  bool operator!=(const DerivedIterator& other) const noexcept {
    return !static_cast<const DerivedIterator*>(this)->trait_equals(other);
  }

  DerivedIterator& operator++() noexcept {
    return static_cast<DerivedIterator*>(this)->trait_next();
  }

  DerivedIterator operator++(int) noexcept {
    auto tmp = *this;
    static_cast<DerivedIterator*>(this)->trait_next();
    return tmp;
  }
};

template <typename Derived, typename DerivedIterator>
class IteratorTrait {
 protected:
  IteratorTrait() = default;
  ~IteratorTrait() = default;

 public:
  DerivedIterator begin() const noexcept {
    return static_cast<const Derived*>(this)->trait_begin();
  }

  DerivedIterator end() const noexcept {
    return static_cast<const Derived*>(this)->trait_end();
  }
};

}  // namespace PawnDB

#endif
