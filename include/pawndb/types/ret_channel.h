#ifndef PAWNDB_TYPES_RETURN_CHANNEL_H
#define PAWNDB_TYPES_RETURN_CHANNEL_H

#include <sys/socket.h>

#include <array>
#include <cstddef>
#include <mutex>

#include "pawndb/params.h"
#include "pawndb/result.h"

namespace PawnDB {

/**
 * @brief Thread-safe FIFO channel for tracking dead transactions
 *
 * Features:
 * - Fixed-size circular buffer
 * - Thread synchronization
 * - Size tracking
 * - Non-blocking operations
 */
class RetChannel{
 private:
  std::array<txn_id_t, MAX_TRANSACTIONS> txns_;
  std::size_t head_ = 0;
  std::size_t tail_ = 0;
  std::size_t count_ = 0;
  std::mutex mtx_;

 public:
  enum class ChannelError {
    None,          /**< No error */
    Full,          /**< Channel is full */
    Empty,         /**< Channel is empty */
  };

  using channel_r = Result<txn_id_t, ChannelError>;

  /** @brief Default constructor */
  constexpr RetChannel() noexcept : txns_{}, head_(0), tail_(0), count_(0) {}

  // Non-copyable
  RetChannel(const RetChannel& other) noexcept = delete;
  RetChannel& operator=(const RetChannel& other) noexcept = delete;

  // Channel trait
  /** @brief Send transaction to the channel */
  ChannelError send(const txn_id_t& txn) noexcept;

  /** @brief Receive transaction from the channel */
  channel_r recv() noexcept;

  // Size trait
  /** @brief Get current transaction count */
  std::size_t size() const noexcept;

  // Container trait
  /** @brief Check if channel is full */
  bool full() const noexcept;

  /** @brief Check if channel is empty */
  bool empty() const noexcept;

  /** @brief Clear all transactions */
  void clear() noexcept;
};

}  // namespace PawnDB

#endif
