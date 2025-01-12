#ifndef PAWNDB_TYPES_RETURN_CHANNEL_H
#define PAWNDB_TYPES_RETURN_CHANNEL_H

#include <sys/socket.h>

#include <array>
#include <cstddef>
#include <mutex>

#include "pawndb/params.h"
#include "pawndb/traits/container.h"
#include "pawndb/traits/queue.h"
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
class RetChannel : public QueueTrait<RetChannel, txn_id_t>,
                   public SizedTrait<RetChannel>,
                   public ContainerTrait<RetChannel> {
  std::array<txn_id_t, MAX_TRANSACTIONS> txns_;
  std::size_t head_ = 0;
  std::size_t tail_ = 0;
  std::size_t count_ = 0;
  std::mutex mtx_;

 public:
  /** @brief Default constructor */
  RetChannel() noexcept;

  // Non-copyable
  RetChannel(const RetChannel& other) noexcept = delete;
  RetChannel& operator=(const RetChannel& other) noexcept = delete;

  // Non-movable
  RetChannel(RetChannel&& other) noexcept = delete;
  RetChannel& operator=(RetChannel&& other) noexcept = delete;

  // Queue operations
  /** @brief Get next transaction without blocking */
  queue_r trait_get() noexcept;

  /** @brief Wait for and get next transaction */
  queue_r trait_recv() noexcept;

  /** @brief Add dead transaction */
  QueueError trait_send(const txn_id_t& txn) noexcept;

  /** @brief Remove front transaction */
  void trait_pop() noexcept;

  /** @brief Clear all transactions */
  void trait_clear() noexcept;

  /** @brief Signal not empty condition */
  void trait_notify_not_empty() noexcept;

  // Size tracking
  /** @brief Get current transaction count */
  std::size_t trait_size() const noexcept;

  /** @brief Check if channel is full */
  bool trait_full() const noexcept;

  /** @brief Check if channel is empty */
  bool trait_empty() const noexcept;
};

}  // namespace PawnDB

#endif
