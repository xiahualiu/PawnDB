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
 * @brief Client connection information
 *
 * Contains client address and length for socket communication.
 * Used to track client connections in job processing.
 */
struct ClientInfo {
  sockaddr_un addr;   /**< Client address */
  socklen_t addr_len; /**< Address length */
};

/**
 * @brief Database job containing client request data
 *
 * Features:
 * - Buffer storage for request data
 * - Client connection tracking
 * - Copy operations for job passing
 */
class Job {
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

  /** @brief Get buffer reference */
  BufferRef buf() const noexcept;

  /** @brief Get buffer size */
  std::size_t buf_size() const noexcept;

  /** @brief Get client address */
  const ClientInfo& client_info() const noexcept;

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
class JobChannel {
  /** @brief Maximum jobs per channel */
  constexpr static std::size_t MaxJobs = MAX_JOB_CHANNEL;

 public:
  /** @brief Channel error codes */
  enum class Error {
    None,    /**< Operation successful */
    Full,    /**< Channel is full */
    Empty,   /**< Channel is empty */
    Timeout, /**< Operation timed out */
  };

  /** @brief Result type for job operations */
  using channel_r = Result<Job, Error>;

  /** @brief Initialize empty channel */
  JobChannel() noexcept;

  // Non-copyable
  JobChannel(const JobChannel& other) noexcept = delete;
  JobChannel& operator=(const JobChannel& other) noexcept = delete;

  /** @brief Send job to channel */
  Error send(const Job& _job) noexcept;

  /** @brief Wait for and get next job */
  channel_r recv() noexcept;

  /** @brief Notify waiting threads */
  void notify() noexcept;

  /** @brief Get job count */
  std::size_t size() const noexcept;

  /** @brief Check if full */
  bool full() const noexcept;

  /** @brief Check if empty */
  bool empty() const noexcept;

  /** @brief Clear job channel */
  void clear() noexcept;

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
