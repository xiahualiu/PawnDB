#ifndef PAWNDB_TRAITS_CHANNEL_H
#define PAWNDB_TRAITS_CHANNEL_H

#include "pawndb/result.h"

namespace PawnDB {

enum class ChannelError { None, Full, Empty, Timeout };

/** @brief Channel trait for implementing messaging interfaces
 * @tparam Derived The derived class implementing this trait
 * @tparam T The type of data being sent/received
 *
 * - `channel_r trait_send(const T& value) noexcept`
 * - `channel_r trait_recv(T& value) noexcept` */
template <typename Derived, typename T>
class ChannelTrait {
 public:
  using value_type = T;
  using channel_r = Result<T, ChannelError>;

 public:
  /** @brief Send data through the channel
   * @param value Data to send
   * @return Result indicating success or error */
  ChannelError send(const T& value) noexcept {
    return derived().trait_send(value);
  }

  /** @brief Receive data from the channel
   * @return Result indicating success or error */
  channel_r recv() noexcept {
    return derived().trait_recv();
  }

  /** @brief Notify waiting threads */
  void notify() noexcept {
    derived().trait_notify();
  }

 protected:
  // Hide constructors
  ChannelTrait() = default;
  ~ChannelTrait() = default;

  // CRTP helpers
  Derived& derived() {
    return static_cast<Derived&>(*this);
  }
  const Derived& derived() const {
    return static_cast<const Derived&>(*this);
  }
};

}  // namespace PawnDB

#endif  // PAWNDB_TRAITS_CHANNEL_H
