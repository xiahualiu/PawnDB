#ifndef PAWNDB_TYPES_JOB_CHANNEL_H
#define PAWNDB_TYPES_JOB_CHANNEL_H

#include <array>
#include <condition_variable>
#include <cstddef>
#include <mutex>

#include "pawndb/params.h"
#include "pawndb/result.h"
#include "pawndb/types/job.h"

namespace PawnDB {

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
