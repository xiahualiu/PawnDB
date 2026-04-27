#ifndef PAWNDB_TYPES_JOB_CHANNEL_H
#define PAWNDB_TYPES_JOB_CHANNEL_H

#include <sys/socket.h>
#include <sys/un.h>

#include <array>
#include <condition_variable>
#include <cstddef>
#include <mutex>

#include "pawndb/params.h"
#include "pawndb/result.h"
#include "pawndb/types/buffer_table.h"

namespace PawnDB {

/**
 * @brief Database job containing client request data
 *
 * Features:
 * - Buffer storage for request data
 * - Client connection tracking
 * - Copy operations for job passing
 */
class job {
 public:
  /** @brief Initialize empty job */
  constexpr job() noexcept
      : buffer_(), buffer_size_(0), client_addr_(), client_addr_len_(0) {}

  /** @brief Initialize job with buffer and client address */
  job(buf_ref _buffer, std::size_t _buffer_size, const sockaddr_un& _addr,
      socklen_t _addr_len) noexcept;

  // Copyable
  job(const job& other) noexcept;
  job& operator=(const job& other) noexcept;

  /** @brief Get buffer reference */
  buf_ref buf() const noexcept;

  /** @brief Get buffer size */
  std::size_t buf_sz() const noexcept;

  /** @brief Get client address */
  const sockaddr* c_addr() const noexcept;

  /** @brief Get address length */
  socklen_t c_addr_len() const noexcept;

 private:
  buf_ref buffer_;            /**< Request data buffer */
  std::size_t buffer_size_;   /**< Request data size */
  sockaddr_un client_addr_;   /**< Client address */
  socklen_t client_addr_len_; /**< Address length */
};

/**
 * @brief Thread-safe FIFO channel for database jobs.
 *
 * Features:
 * - Fixed-size circular buffer
 * - Blocking and non-blocking operations
 * - Thread synchronization
 * - Size tracking
 */
class job_channel {
 public:
  /** @brief Result type for queue operations */
  using queue_r = Result<job, QueueError>;

 private:
  /** @brief Maximum jobs per channel */
  constexpr static std::size_t MaxJobs = MAX_ITEM_PER_CHANNEL;

 public:
  /** @brief Initialize empty channel */
  job_channel() noexcept;

  // Non-copyable
  job_channel(const job_channel& other) noexcept = delete;
  job_channel& operator=(const job_channel& other) noexcept = delete;

  /** @brief Non-blocking get */
  queue_r get() noexcept;

  /** @brief Blocking receive */
  queue_r recv() noexcept;

  /** @brief Send job */
  QueueError send(const job& job) noexcept;

  /** @brief Remove front job */
  void pop() noexcept;

  /** @brief Clear all jobs */
  void clear() noexcept;

  /** @brief Signal not empty */
  void notify_not_empty() noexcept;

  /** @brief Get job count */
  std::size_t size() const noexcept;

  /** @brief Check if full */
  bool full() const noexcept;

  /** @brief Check if empty */
  bool empty() const noexcept;

 private:
  std::array<job, MaxJobs> jobs_;     /**< Job storage */
  std::size_t head_;                  /**< Read position */
  std::size_t tail_;                  /**< Write position */
  std::size_t count_;                 /**< Current job count */
  std::mutex mtx_;                    /**< Thread safety */
  std::condition_variable not_empty_; /**< Empty signal */
};

}  // namespace PawnDB

#endif
