#ifndef PAWNDB_TYPES_JOB_CHANNEL_H
#define PAWNDB_TYPES_JOB_CHANNEL_H

#include <sys/socket.h>
#include <sys/un.h>

#include <array>
#include <condition_variable>
#include <cstddef>
#include <mutex>

#include "pawndb/params.h"
#include "pawndb/traits/channel.h"
#include "pawndb/traits/container.h"
#include "pawndb/traits/copy.h"
#include "pawndb/traits/job.h"
#include "pawndb/traits/sized.h"
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
class Job : public JobTrait<Job>, public CopyTrait<Job> {
 public:
  /** @brief Initialize empty job */
  constexpr Job() noexcept
      : buffer_(), buffer_size_(0), client_addr_(), client_addr_len_(0) {}

  /** @brief Initialize job with buffer and client address */
  Job(BufferRef _buffer, std::size_t _buffer_size, const sockaddr_un& _addr,
      socklen_t _addr_len) noexcept;

  // Copyable
  Job(const Job& other) noexcept;
  Job& operator=(const Job& other) noexcept;

  // JobTrait Implementation
  /** @brief Get buffer reference */
  BufferRef trait_buf() const noexcept;

  /** @brief Get buffer size */
  std::size_t trait_buf_size() const noexcept;

  /** @brief Get client address */
  const sockaddr* trait_c_addr() const noexcept;

  /** @brief Get address length */
  socklen_t trait_c_addr_len() const noexcept;

 private:
  BufferRef buffer_;          /**< Request data buffer */
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
 *
 * Implemented traits:
 * - DequeTrait: FIFO operations
 * - ChannelTrait: Blocking operations
 * - SizedTrait: Size tracking
 * - ContainerTrait: Capacity operations
 */
class JobChannel : public ChannelTrait<JobChannel, Job>,
                   public SizedTrait<JobChannel>,
                   public ContainerTrait<JobChannel> {
  /** @brief Maximum jobs per channel */
  constexpr static std::size_t MaxJobs = MAX_JOB_CHANNEL;

 public:
  /** @brief Initialize empty channel */
  JobChannel() noexcept;

  // Non-copyable
  JobChannel(const JobChannel& other) noexcept = delete;
  JobChannel& operator=(const JobChannel& other) noexcept = delete;

  // ChannelTrait Implementation
  /** @brief Send job to channel */
  ChannelError trait_send(const Job& _job) noexcept;

  /** @brief Wait for and get next job */
  channel_r trait_recv() noexcept;

  /** @brief Notify waiting threads */
  void trait_notify() noexcept;

  // SizedTrait Implementation
  /** @brief Get job count */
  std::size_t trait_size() const noexcept;

  // ContainerTrait Implementation
  /** @brief Check if full */
  bool trait_full() const noexcept;

  /** @brief Check if empty */
  bool trait_empty() const noexcept;

  /** @brief Clear job channel */
  void trait_clear() noexcept;

 private:
  std::array<Job, MaxJobs> jobs_;     /**< Job storage */
  std::size_t head_;                  /**< Read position */
  std::size_t tail_;                  /**< Write position */
  std::size_t count_;                 /**< Current job count */
  std::mutex mtx_;                    /**< Thread safety */
  std::condition_variable not_empty_; /**< Empty signal */
};

}  // namespace PawnDB

#endif
