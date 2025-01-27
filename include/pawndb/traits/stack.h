#ifndef PAWNDB_TRAITS_STACK_H
#define PAWNDB_TRAITS_STACK_H

#include "pawndb/result.h"

namespace PawnDB {

/**
 * @brief Queue operation error codes
 */
enum class StackError {
  None,  /**< Operation successful */
  Empty, /**< Queue has no data */
  Full,  /**< Queue at capacity */
};

/**
 * @brief CRTP base class providing stack operations.
 *
 * This class uses the Curiously Recurring Template Pattern (CRTP) to
 * define common stack operations (`push`, `pop`, `top`) that delegate
 * to the derived class implementations.
 *
 * @tparam Derived The derived class implementing the actual stack operations.
 * @tparam T The type of elements stored in the stack.
 */
template <typename Derived, typename T>
class StackTrait {
 public:
  /** @brief Value type stored in queue */
  using value_type = T;

  /** @brief Get operation result type */
  using queue_r = Result<value_type&, StackError>;

  /**
   * @brief Accesses the top element of the stack.
   * @return T& Reference to the top element of the stack.
   */
  T& top() {
    return static_cast<Derived*>(this)->trait_top();
  }

  /**
   * @brief Pushes a value onto the stack.
   * @param value The value to be pushed onto the stack.
   */
  void push(const T& value) {
    static_cast<Derived*>(this)->trait_push(value);
  }

  /**
   * @brief Removes the top element from the stack.
   */
  void pop() {
    static_cast<Derived*>(this)->trait_pop();
  }
};

}  // namespace PawnDB

#endif
