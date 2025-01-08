#ifndef PAWNDB_TYPES_JOB_CHANNEL_H
#define PAWNDB_TYPES_JOB_CHANNEL_H

#include <sys/socket.h>

#include <array>
#include <condition_variable>
#include <cstddef>
#include <mutex>

#include "pawndb/params.h"
#include "pawndb/traits/container.h"
#include "pawndb/traits/fifo.h"
#include "pawndb/traits/sized.h"

namespace PawnDB {

struct Job {
  std::size_t buffer_index_;
  std::size_t buffer_size_;
  struct sockaddr client_addr_;
  socklen_t client_addr_len_;
};

class JobChannel : public FIFO<JobChannel, Job>,
                   public Sized<JobChannel>,
                   public Container<JobChannel> {
  constexpr static std::size_t MaxJobs = MAX_ITEM_PER_CHANNEL;
  std::array<Job, MaxJobs> jobs_;
  std::size_t head_;
  std::size_t tail_;
  std::size_t count_;
  std::mutex mtx_;
  std::condition_variable not_empty_;

 public:
  JobChannel() noexcept : jobs_(), head_(0), tail_(0), count_(0) {}

  std::size_t trait_size() const noexcept { return count_; }
  std::size_t constexpr trait_capacity() const noexcept { return MaxJobs; }
  bool trait_full() const noexcept { return count_ == MaxJobs; }
  bool trait_empty() const noexcept { return count_ == 0; }

  GetR trait_get() noexcept {
    std::unique_lock<std::mutex> lock(mtx_);
    if (!empty()) {
      return jobs_[head_];
    } else {
      return FIFOError::Empty;
    }
  }

  GetR trait_recv() noexcept {
    std::unique_lock<std::mutex> lock(mtx_);
    auto w_r =
        not_empty_.wait_for(lock, WAIT_TIMEOUT, [this] { return !empty(); });
    if (w_r) {
      return jobs_[head_];
    } else {
      return FIFOError::Timeout;
    }
  }

  FIFOError trait_send(const Job& job) noexcept {
    std::unique_lock<std::mutex> lock(mtx_);
    if (full()) {
      return FIFOError::Full;
    }
    jobs_[tail_] = job;
    tail_ = (tail_ + 1) % MaxJobs;
    count_++;
    return FIFOError::None;
  }

  FIFOError trait_send(Job&& job) noexcept {
    std::unique_lock<std::mutex> lock(mtx_);
    if (full()) {
      return FIFOError::Full;
    }
    jobs_[tail_] = std::move(job);
    tail_ = (tail_ + 1) % MaxJobs;
    count_++;
    return FIFOError::None;
  }

  void trait_pop() noexcept {
    std::unique_lock<std::mutex> lock(mtx_);
    if (!empty()) {
      head_ = (head_ + 1) % MaxJobs;
      count_--;
    }
  }

  void trait_clear() noexcept {
    std::unique_lock<std::mutex> lock(mtx_);
    head_ = 0;
    tail_ = 0;
    count_ = 0;
  }

  void trait_notify_not_empty() noexcept { not_empty_.notify_one(); }
};

}  // namespace PawnDB

#endif
