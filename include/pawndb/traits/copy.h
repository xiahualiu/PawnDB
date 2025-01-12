#ifndef PAWNDB_TRAITS_COPY_H
#define PAWNDB_TRAITS_COPY_H

namespace PawnDB {

/**
 * @brief CRTP base class for copyable objects
 * @note Derived class usually has copy constructor and copy assignment operator
 * defined from this trait
 * @tparam Derived The derived class implementing copy operations
 *
 * Provides interface for deep copying and cloning objects.
 *
 * Required trait implementations:
 * - trait_copy(const Derived&) -> void : Deep copy from source
 * - trait_clone() const -> Derived : Create new copy of this
 */
template <typename Derived>
class CopyTrait {
 public:
  /**
   * @brief Copy from another instance
   * @param _other Source instance to copy from
   * @post This instance becomes deep copy of other
   */
  void copy(const Derived& _other) noexcept {
    static_cast<Derived*>(this)->trait_copy(_other);
  }

  /**
   * @brief Create clone of current instance
   * @return New copy of current instance
   * @post New instance is deep copy of this
   */
  Derived clone() const noexcept {
    return static_cast<const Derived*>(this)->trait_clone();
  }

 protected:
  CopyTrait() = default;
  ~CopyTrait() = default;
};

}  // namespace PawnDB

#endif  // PAWNDB_TRAITS_COPY_H
