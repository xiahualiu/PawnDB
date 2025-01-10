
#ifndef PAWNDB_TRAITS_CHANNEL_H
#define PAWNDB_TRAITS_CHANNEL_H

#include "pawndb/result.h"

namespace PawnDB {
/**
 * @brief Channel operation error codes
 */
enum class FIFOError {
  None,   /**< Operation successful */
  Empty,  /**< Channel has no data */
  Full,   /**< Channel at capacity */
  Timeout /**< Operation timed out */
};

/**
 * @brief CRTP base class implementing FIFO queue interface
 * @tparam Derived The derived class implementing the trait methods
 * @tparam T Type of elements stored in the FIFO queue
 *
 * Required trait implementations:
 * - trait_get()
 * - trait_recv()
 * - trait_send(const value_type&)
 * - trait_send(value_type&&)
 * - trait_clear()
 * - trait_pop()
 * - trait_notify_not_empty()
 */
template <typename Derived, typename T>
class FIFO {
 public:
  using value_type = T;
  using GetR = Result<value_type, FIFOError>;

  /**
   * @brief Get next value from FIFO without blocking
   * @return Result containing either value or error
   */
  GetR get() noexcept { return static_cast<Derived*>(this)->trait_get(); }

  /**
   * @brief Get next value from FIFO with blocking
   * @return Result containing either value or error
   */
  GetR recv() noexcept { return static_cast<Derived*>(this)->trait_recv(); }

  /**
   * @brief Send value to FIFO
   * @param value The value to send
   * @return Error status of the operation
   */
  FIFOError send(const value_type& value) noexcept {
    return static_cast<Derived*>(this)->trait_send(value);
  }

  /**
   * @brief Send value to FIFO using move semantics
   * @param value The value to send
   * @return Error status of the operation
   */
  FIFOError send(value_type&& value) noexcept {
    return static_cast<Derived*>(this)->trait_send(std::move(value));
  }

  /**
   * @brief Clear all elements from FIFO
   */
  void clear() noexcept { static_cast<Derived*>(this)->trait_clear(); }

  /**
   * @brief Remove next element from FIFO
   */
  void pop() noexcept { static_cast<Derived*>(this)->trait_pop(); }

  /**
   * @brief Signal that FIFO is not empty
   */
  void notify_not_empty() noexcept {
    static_cast<Derived*>(this)->trait_notify_not_empty();
  }

 protected:
  FIFO() = default;
  ~FIFO() = default;
};

}  // namespace PawnDB

#endif  // PAWNDB_TRAITS_CHANNEL_H
