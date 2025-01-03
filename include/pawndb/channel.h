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

enum class ChannelError { None, GetNothing, Timeout, TableFull };

template <typename T>
class Channel : private Queue<T, CHANNEL_ROWS> {
 private:
  using _base = Queue<T, CHANNEL_ROWS>;

 public:
  using _base::empty;
  using _base::full;

  Channel() = default;

  using GetR = Result<T, ChannelError>;

  GetR get() noexcept {
    auto lock = std::unique_lock<std::mutex>(ch_mutex);
    if (empty()) {
      return ChannelError::GetNothing;
    } else {
      return this->front();
    }
  }

  GetR recv() noexcept {
    auto lock = std::unique_lock<std::mutex>(ch_mutex);
    if (not_empty.wait_for(lock, WAIT_TIMEOUT, [this] { return !empty(); })) {
      return this->front();
    } else {
      return ChannelError::Timeout;
    }
  }

  void pop() noexcept {
    auto lock = std::unique_lock<std::mutex>(ch_mutex);
    _base::pop();
  }

  ChannelError send(const T& _value) noexcept {
    auto lock = std::unique_lock<std::mutex>(ch_mutex);
    if (full()) {
      return ChannelError::TableFull;
    } else {
      _base::push(_value);
      return ChannelError::None;
    }
  }

  ChannelError send(T&& _value) noexcept {
    auto lock = std::unique_lock<std::mutex>(ch_mutex);
    if (full()) {
      return ChannelError::TableFull;
    } else {
      _base::push(std::move(_value));
      return ChannelError::None;
    }
  }

  void clear() noexcept {
    auto lock = std::unique_lock<std::mutex>(ch_mutex);
    _base::clear();
  }

  inline void notify() noexcept { not_empty.notify_one(); }

 private:
  std::mutex ch_mutex;
  std::condition_variable not_empty;
};

}  // namespace PawnDB

#endif  // PAWNDB_THREAD_CHANNEL_H
