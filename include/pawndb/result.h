/**
 * @file result.h
 * @author Xiahua Liu @xiahualiu
 * @brief PawnDB result type. PawnDB doesn't use exceptions.
 * @version 0.1
 * @date 2025-01-02
 *
 * @copyright MIT License
 *
 */

#ifndef PAWNDB_RESULT_H
#define PAWNDB_RESULT_H

#include <type_traits>

namespace PawnDB {

template <typename T, typename E>
class Result {
  static_assert(!std::is_same_v<T, E>, "T and E cannot be the same type!");
  static_assert(!std::is_reference_v<T>, "T cannot be a reference!");
  static_assert(std::is_enum_v<E>, "E must be an enum type!");

 public:
  Result() = delete;
  Result(const T& _value) noexcept : value(_value), error(E::None) {}
  Result(const T&& _value) noexcept : value(_value), error(E::None) {}
  Result(const E& _error) noexcept : error(_error) {}

  explicit operator bool() const noexcept { return error == E::None; }
  inline bool operator!() const noexcept { return error != E::None; }

  inline T& unwrap() noexcept { return value; }
  inline const T& unwrap() const noexcept { return value; }

  inline const E getError() const noexcept { return error; }

 private:
  T value;
  E error;
};
}  // namespace PawnDB

#endif  // PAWNDB_RESULT_H
