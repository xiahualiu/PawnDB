#ifndef PAWNDB_TYPES_RET_CHANNEL_H
#define PAWNDB_TYPES_RET_CHANNEL_H

#include <array>
#include <condition_variable>
#include <cstddef>
#include <mutex>

#include "pawndb/params.h"
#include "pawndb/types/ret.h"

namespace PawnDB {

/**
 * @brief Thread-safe FIFO channel for database return messages.
 *
 * Features:
 * - Fixed-size circular buffer
 * - Blocking and non-blocking operations
 * - Thread synchronization
 * - Size tracking
 */
class ret_channel {
 public:
  /** @brief Result type for queue operations */
  using queue_r = Result<ret, QueueError>;

 private:
  /** @brief Maximum returns per channel */
  constexpr static std::size_t MaxRets = MAX_ITEM_PER_CHANNEL;

 public:
  /** @brief Initialize empty channel */
  ret_channel() noexcept;

  // Non-copyable
  ret_channel(const ret_channel& other) noexcept = delete;
  ret_channel& operator=(const ret_channel& other) noexcept = delete;

  /** @brief Non-blocking get */
  queue_r get() noexcept;

  /** @brief Blocking receive */
  queue_r recv() noexcept;

  /** @brief Send return */
  QueueError send(const ret& _ret) noexcept;

  /** @brief Remove front return */
  void pop() noexcept;

  /** @brief Clear all returns */
  void clear() noexcept;

  /** @brief Signal not empty */
  void notify_not_empty() noexcept;

  /** @brief Get return count */
  std::size_t size() const noexcept;

  /** @brief Check if full */
  bool full() const noexcept;

  /** @brief Check if empty */
  bool empty() const noexcept;

 private:
  std::array<ret, MaxRets> rets_;     /**< Return storage */
  std::size_t head_;                  /**< Read position */
  std::size_t tail_;                  /**< Write position */
  std::size_t count_;                 /**< Current return count */
  std::mutex mtx_;                    /**< Thread safety */
  std::condition_variable not_empty_; /**< Empty signal */
};

}  // namespace PawnDB

#endif
