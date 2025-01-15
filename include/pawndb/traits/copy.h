#ifndef PAWNDB_TRAITS_COPY_H
#define PAWNDB_TRAITS_COPY_H

namespace PawnDB {

template<typename Derived>
class CopyTrait {
 public:
  constexpr static bool is_copyable = true;

  /** @brief Create a copy of this object */
  Derived clone() const noexcept {
    return static_cast<const Derived*>(this)->trait_clone();
  }

  /** @brief Copy from another object */
  void copy(const Derived& other) noexcept {
    static_cast<Derived*>(this)->trait_copy(other);
  }

 protected:
  CopyTrait() = default;
  ~CopyTrait() = default;
};

}  // namespace PawnDB

#endif  // PAWNDB_TRAITS_COPY_H
