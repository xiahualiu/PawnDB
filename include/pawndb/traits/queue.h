#ifndef PAWNDB_TRAITS_QUEUE_H
#define PAWNDB_TRAITS_QUEUE_H

#include "pawndb/result.h"

namespace PawnDB {

/**
 * @brief Queue operation error codes
 */
enum class QueueError {
  None,   /**< Operation successful */
  Empty,  /**< Queue has no data */
  Full,   /**< Queue at capacity */
  Timeout /**< Operation timed out */
};

/**
 * @brief CRTP base class for queue implementations
 * @tparam Derived The derived queue class
 * @tparam T Type of elements stored in queue
 *
 * Required trait implementations:
 * - trait_get() -> GetR : Non-blocking get
 * - trait_recv() -> GetR : Blocking get
 * - trait_send(const T&) -> QueueError : Send value
 * - trait_clear() -> void : Clear queue
 * - trait_pop() -> void : Remove front element
 */
template <typename Derived, typename T>
class QueueTrait {
 public:
  /** @brief Value type stored in queue */
  using value_type = T;

  /** @brief Get operation result type */
  using queue_r = Result<value_type&, QueueError>;

  /**
   * @brief Get next value from queue without blocking
   * @return Result containing either value or error
   */
  queue_r get() noexcept { return static_cast<Derived*>(this)->trait_get(); }

  /**
   * @brief Get next value from queue with blocking
   * @return Result containing either value or error
   */
  queue_r recv() noexcept { return static_cast<Derived*>(this)->trait_recv(); }

  /**
   * @brief Copy value to FIFO
   * @param _value The value to send
   * @return Error status of the operation
   */
  QueueError send(const value_type& _value) noexcept {
    return static_cast<Derived*>(this)->trait_send(_value);
  }

  /**
   * @brief Clear all elements from queue
   */
  void clear() noexcept { static_cast<Derived*>(this)->trait_clear(); }

  /**
   * @brief Remove next element from queue
   */
  void pop() noexcept { static_cast<Derived*>(this)->trait_pop(); }

  /**
   * @brief Signal that queue is not empty
   */
  void notify_not_empty() noexcept {
    static_cast<Derived*>(this)->trait_notify_not_empty();
  }

  /**
   * @brief Signal that queue is not full
   */
  void notify_not_full() noexcept {
    static_cast<Derived*>(this)->trait_notify_not_full();
  }

 protected:
  QueueTrait() = default;
  ~QueueTrait() = default;
};

}  // namespace PawnDB

#endif  // PAWNDB_TRAITS_CHANNEL_H
