#ifndef PAWNDB_TRAITS_QUEUE_H
#define PAWNDB_TRAITS_QUEUE_H

#include "pawndb/result.h"

namespace PawnDB {

/**
 * @brief Queue operation error codes
 */
enum class QueueError {
  None,  /**< Operation successful */
  Empty, /**< Queue has no data */
  Full,  /**< Queue at capacity */
};

/**
 * @brief CRTP base class providing queue operations.
 *
 * @tparam Derived The derived class implementing the actual queue operations.
 * @tparam T The type of elements stored in the queue.
 */
template <typename Derived, typename T>
class QueueTrait {
 public:
  /** @brief Value type stored in queue */
  using value_type = T;

  /** @brief Get operation result type */
  using queue_r = Result<value_type&, QueueError>;

  /**
   * @brief Retrieves the next value from the queue without blocking.
   * @return Result containing either a reference to the front value or an error
   * code.
   */
  queue_r front() noexcept {
    return static_cast<Derived*>(this)->trait_front();
  }

  /**
   * @brief Adds a new value to the queue.
   *
   * @param _value The value to be added to the queue.
   * @return Result indicating success or the appropriate error code.
   */
  queue_r push(const T& _value) noexcept {
    return static_cast<Derived*>(this)->trait_push(_value);
  }

  /**
   * @brief Clear all elements from queue
   */
  void clear() noexcept {
    static_cast<Derived*>(this)->trait_clear();
  }

  /**
   * @brief Removes the next element from the queue.
   * @return Result indicating success or the appropriate error code.
   */
  QueueError pop() noexcept {
    static_cast<Derived*>(this)->trait_pop();
  }

 protected:
  // Protected constructor and destructor
  QueueTrait() = default;
  ~QueueTrait() = default;
};

}  // namespace PawnDB

#endif  // PAWNDB_TRAITS_CHANNEL_H
