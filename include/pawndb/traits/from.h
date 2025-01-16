#ifndef PAWNDB_TRAITS_FROM_H
#define PAWNDB_TRAITS_FROM_H

namespace PawnDB {

/**
 * @brief Trait for converting from source type
 * @tparam To Target type to convert to
 * @tparam From Source type to convert from
 */
template <typename Derived, typename From>
class FromTrait {
 public:
  /** @brief Convert from source type */
  void from(const From& value) noexcept {
    return static_cast<Derived*>(this)->trait_from(value);
  }

 protected:
  FromTrait() = default;
  ~FromTrait() = default;
};

}  // namespace PawnDB

#endif  // PAWNDB_TRAITS_FROM_H