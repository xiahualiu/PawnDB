/**
 * @file fifo.h
 * @brief CRTP interface for thread-safe channel communication
 * @version 0.1
 * @date 2025-01-02
 *
 * Features:
 * - Thread-safe send/receive
 * - Blocking and non-blocking operations
 * - Move semantics support
 * - Timeout handling
 */
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
 * @brief CRTP interface for channel implementations
 * @tparam Derived Class implementing the channel interface
 * @tparam T Type of values transmitted through channel
 *
 * Required implementations:
 * - GetR trait_get()
 * - GetR trait_recv()
 * - FIFOError trait_send(const T&)
 * - FIFOError trait_send(T&&)
 */
template <typename Derived, typename T>
class FIFO {
 public:
  using value_type = T;
  using GetR = Result<value_type, FIFOError>;

  /**
   * @brief Non-blocking get operation
   * @return GetR Success: value, Error: Empty
   */
  GetR get() noexcept { return static_cast<Derived*>(this)->trait_get(); }

  /**
   * @brief Blocking receive with timeout
   * @return GetR Success: value, Error: Timeout
   */
  GetR recv() noexcept { return static_cast<Derived*>(this)->trait_recv(); }

  /**
   * @brief Send value to channel (copy)
   * @param value Value to send
   * @return FIFOError None or Full
   */
  FIFOError send(const value_type& value) noexcept {
    return static_cast<Derived*>(this)->trait_send(value);
  }

  /**
   * @brief Send value to channel (move)
   * @param value Value to move
   * @return FIFOError None or Full
   */
  FIFOError send(value_type&& value) noexcept {
    return static_cast<Derived*>(this)->trait_send(std::move(value));
  }

  /**
   * @brief Clear all contents from channel
   * @details Thread-safe removal of all elements
   */
  void clear() noexcept { static_cast<Derived*>(this)->trait_clear(); }

  /**
   * @brief Removes an element from the channel.
   *
   * This function calls the `trait_pop` method of the derived class to
   * remove an element from the channel. It is marked as `noexcept` to
   * indicate that it does not throw any exceptions.
   */
  void pop() noexcept { static_cast<Derived*>(this)->trait_pop(); }

  /**
   * @brief Notifies that the channel is not empty.
   *
   * This function is intended to be called when the channel transitions
   * from empty to not empty. It delegates the actual notification logic
   * to the derived class by calling `trait_notify_not_empty()` on the
   * derived class instance.
   *
   * @note This function is `noexcept`, meaning it guarantees not to throw
   * any exceptions.
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
