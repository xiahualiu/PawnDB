/**
 * @file td_channel.h
 * @author Xiahua Liu @xiahualiu
 * @brief Thread channel class, used for async communication between threads.
 * @version 0.1
 * @date 2025-01-02
 *
 * @copyright MIT License
 *
 */

#ifndef PAWNDB_THREAD_CHANNEL_H
#define PAWNDB_THREAD_CHANNEL_H

#include <condition_variable>
#include <mutex>

#include "pawndb/ds/queue.h"
#include "pawndb/params.h"
#include "pawndb/result.h"

namespace PawnDB {

/**
 * @brief Enumeration of possible channel errors.
 */
enum class ChannelError {
  None,       /**< No error */
  GetNothing, /**< No item to get */
  Timeout,    /**< Operation timed out */
  TableFull,  /**< Channel is full */
};

/**
 * @brief Channel data structure.
 *
 * @tparam T The type of the elements in the channel.
 */
template <typename T>
class Channel : private Queue<T, CHANNEL_ROWS> {
 private:
  using _base = Queue<T, CHANNEL_ROWS>;

 public:
  using _base::empty;
  using _base::full;

  Channel() = default;

  using GetR = Result<T, ChannelError>;

  /**
   * @brief Gets an element from the channel without blocking.
   *
   * @return GetR The element if available, or a ChannelError if not.
   */
  GetR get() noexcept {
    auto lock = std::unique_lock<std::mutex>(ch_mutex);
    if (empty()) {
      return ChannelError::GetNothing;
    } else {
      return this->front();
    }
  }

  /**
   * @brief Receives an element from the channel, blocking until one is
   * available or a timeout occurs.
   *
   * @return GetR The element if available, or a ChannelError if not.
   */
  GetR recv() noexcept {
    auto lock = std::unique_lock<std::mutex>(ch_mutex);
    if (not_empty.wait_for(lock, WAIT_TIMEOUT, [this] { return !empty(); })) {
      return this->front();
    } else {
      return ChannelError::Timeout;
    }
  }

  /**
   * @brief Pops an element from the channel.
   */
  void pop() noexcept {
    auto lock = std::unique_lock<std::mutex>(ch_mutex);
    _base::pop();
  }

  /**
   * @brief Sends an element to the channel.
   *
   * @param _value The value to send.
   * @return ChannelError::None if the operation is successful,
   * ChannelError::TableFull if the channel is full.
   */
  ChannelError send(const T& _value) noexcept {
    auto lock = std::unique_lock<std::mutex>(ch_mutex);
    if (full()) {
      return ChannelError::TableFull;
    } else {
      _base::push(_value);
      return ChannelError::None;
    }
  }

  /**
   * @brief Sends an element to the channel.
   *
   * @param _value The value to send.
   * @return ChannelError::None if the operation is successful,
   * ChannelError::TableFull if the channel is full.
   */
  ChannelError send(T&& _value) noexcept {
    auto lock = std::unique_lock<std::mutex>(ch_mutex);
    if (full()) {
      return ChannelError::TableFull;
    } else {
      _base::push(std::move(_value));
      return ChannelError::None;
    }
  }

  /**
   * @brief Clears the channel.
   */
  void clear() noexcept {
    auto lock = std::unique_lock<std::mutex>(ch_mutex);
    _base::clear();
  }

  /**
   * @brief Notifies one waiting thread.
   */
  inline void notify() noexcept { not_empty.notify_one(); }

 private:
  std::mutex ch_mutex;
  std::condition_variable not_empty;
};

}  // namespace PawnDB

#endif  // PAWNDB_THREAD_CHANNEL_H
