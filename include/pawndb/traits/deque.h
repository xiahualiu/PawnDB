#ifndef PAWNDB_TRAITS_DEQUE_H
#define PAWNDB_TRAITS_DEQUE_H

#include "pawndb/result.h"

namespace PawnDB {

/**
 * @brief Queue operation error codes
 */
enum class DequeError {
  None,  /**< Operation successful */
  Empty, /**< Queue has no data */
  Full,  /**< Queue at capacity */
};

/** @brief CRTP base class for deque implementations
 * @tparam Derived The derived deque class
 * @tparam T The value type stored in the deque
 *
 * Required trait implementations:
 * - trait_front() -> Result<T&, DequeError>
 * - trait_front() -> Result<T, DequeError>
 * - trait_back() -> Result<T&, DequeError>
 * - trait_back() -> Result<T, DequeError>
 * - trait_push_front(const T&) -> DequeError
 * - trait_push_back(const T&) -> DequeError
 * - trait_pop_front() -> DequeError
 * - trait_pop_back() -> DequeError
 */
template <typename Derived, typename T>
class DequeTrait {
 public:
  /** @brief Get operation result type */
  using deque_r = Result<T&, DequeError>;
  using deque_cp_r = Result<T, DequeError>;

  /** @brief Get front element
   * @return Result containing reference to front element or error code */
  deque_r front() noexcept {
    return derived().trait_front();
  }

  /** @brief Get front element
   * @return Result containing copy of front element or error code */
  deque_cp_r front() const noexcept {
    return derived().trait_front();
  }

  /** @brief Get back element
   * @return Result containing reference to back element or error code */
  deque_r back() noexcept {
    return derived().trait_back();
  }

  /** @brief Get back element
   * @return Result containing copy of back element or error code */
  deque_cp_r back() const noexcept {
    return derived().trait_back();
  }

  /** @brief Add element to front of deque
   * @param _value Value to add
   * @return Error code */
  DequeError push_front(const T& _value) noexcept {
    return derived().trait_push_front(_value);
  }

  /** @brief Add element to back of deque
   * @param _value Value to add
   * @return Error code */
  DequeError push_back(const T& _value) noexcept {
    return derived().trait_push_back(_value);
  }

  /** @brief Remove front element
   * @return Error code */
  DequeError pop_front() noexcept {
    derived().trait_pop_front();
  }

  /** @brief Remove back element
   * @return Error code */
  DequeError pop_back() noexcept {
    derived().trait_pop_back();
  }

 protected:
  // Protected constructor and destructor
  DequeTrait() = default;
  ~DequeTrait() = default;

  // CRTP helpers
  Derived& derived() {
    return static_cast<Derived&>(*this);
  }
  const Derived& derived() const {
    return static_cast<const Derived&>(*this);
  }
};

}  // namespace PawnDB

#endif  // PAWNDB_TRAITS_DEQUE_H
