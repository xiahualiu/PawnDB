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

/**
 * @brief Class template representing the result of an operation.
 *
 * @tparam T The type of the value.
 * @tparam E The type of the error, must be an enum.
 */
template <typename T, typename E>
class Result {
  static_assert(!std::is_same_v<T, E>, "T and E cannot be the same type!");
  static_assert(!std::is_reference_v<T>, "T cannot be a reference!");
  static_assert(std::is_enum_v<E>, "E must be an enum type!");

 public:
  /**
   * @brief Deleted default constructor.
   */
  Result() = delete;

  /**
   * @brief Constructs a Result with a value.
   *
   * @param _value The value to construct the Result with.
   */
  Result(const T& _value) noexcept : value(_value), error(E::None) {}

  /**
   * @brief Constructs a Result with a value.
   *
   * @param _value The value to construct the Result with.
   */
  Result(const T&& _value) noexcept : value(_value), error(E::None) {}

  /**
   * @brief Constructs a Result with an error.
   *
   * @param _error The error to construct the Result with.
   */
  Result(const E& _error) noexcept : error(_error) {}

  /**
   * @brief Checks if the Result is successful.
   *
   * @return true if the Result is successful, false otherwise.
   */
  explicit operator bool() const noexcept { return error == E::None; }

  /**
   * @brief Checks if the Result is an error.
   *
   * @return true if the Result is an error, false otherwise.
   */
  inline bool operator!() const noexcept { return error != E::None; }

  /**
   * @brief Unwraps the value from the Result.
   *
   * @return A reference to the value.
   */
  inline T& unwrap() noexcept { return value; }

  /**
   * @brief Unwraps the value from the Result.
   *
   * @return A const reference to the value.
   */
  inline const T& unwrap() const noexcept { return value; }

  /**
   * @brief Gets the error from the Result.
   *
   * @return The error.
   */
  inline const E getError() const noexcept { return error; }

 private:
  T value;
  E error;
};
}  // namespace PawnDB

#endif  // PAWNDB_RESULT_H
