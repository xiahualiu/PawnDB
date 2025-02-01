#ifndef PAWNDB_TYPES_RETURN_CHANNEL_H
#define PAWNDB_TYPES_RETURN_CHANNEL_H

#include <sys/socket.h>

#include <array>
#include <cstddef>
#include <mutex>

#include "pawndb/params.h"
#include "pawndb/traits/channel.h"
#include "pawndb/traits/container.h"
#include "pawndb/traits/deque.h"
#include "pawndb/traits/sized.h"

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
class RetChannel : public DequeTrait<RetChannel, txn_id_t>,
                   public ChannelTrait<RetChannel, txn_id_t>,
                   public SizedTrait<RetChannel>,
                   public ContainerTrait<RetChannel> {
 private:
  std::array<txn_id_t, MAX_TRANSACTIONS> txns_;
  std::size_t head_ = 0;
  std::size_t tail_ = 0;
  std::size_t count_ = 0;
  std::mutex mtx_;

 public:
  /** @brief Default constructor */
  constexpr RetChannel() noexcept : txns_{}, head_(0), tail_(0), count_(0) {}

  // Non-copyable
  RetChannel(const RetChannel& other) noexcept = delete;
  RetChannel& operator=(const RetChannel& other) noexcept = delete;

  // Deque trait
  /** @brief Push transaction to the channel */
  DequeError trait_push_back(const txn_id_t& txn) noexcept;

  /** @brief Get front dead transacion id */
  deque_cp_r trait_front() const noexcept;

  /** @brief Pop transaction from the channel */
  DequeError trait_pop_front() noexcept;

  // Channel trait
  /** @brief Send transaction to the channel */
  ChannelError trait_send(const txn_id_t& txn) noexcept;

  /** @brief Receive transaction from the channel */
  channel_r trait_recv() noexcept;

  // Size trait
  /** @brief Get current transaction count */
  std::size_t trait_size() const noexcept;

  // Container trait
  /** @brief Check if channel is full */
  bool trait_full() const noexcept;

  /** @brief Check if channel is empty */
  bool trait_empty() const noexcept;

  /** @brief Clear all transactions */
  void trait_clear() noexcept;
};

}  // namespace PawnDB

#endif
