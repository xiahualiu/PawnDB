#include "pawndb/types/job_channel.h"

namespace PawnDB {

JobChannel::GetR JobChannel::trait_get() noexcept {
  std::unique_lock<std::mutex> lock(mtx_);
  if (!empty()) {
    return jobs_[head_];
  } else {
    return FIFOError::Empty;
  }
}

JobChannel::GetR JobChannel::trait_recv() noexcept {
  std::unique_lock<std::mutex> lock(mtx_);
  auto w_r =
      not_empty_.wait_for(lock, WAIT_TIMEOUT, [this] { return !empty(); });
  if (w_r) {
    return jobs_[head_];
  } else {
    return FIFOError::Timeout;
  }
}

FIFOError JobChannel::trait_send(const Job& job) noexcept {
  std::unique_lock<std::mutex> lock(mtx_);
  if (full()) {
    return FIFOError::Full;
  }
  jobs_[tail_] = job;
  tail_ = (tail_ + 1) % MaxJobs;
  count_++;
  return FIFOError::None;
}

FIFOError JobChannel::trait_send(Job&& job) noexcept {
  std::unique_lock<std::mutex> lock(mtx_);
  if (full()) {
    return FIFOError::Full;
  }
  jobs_[tail_] = std::move(job);
  tail_ = (tail_ + 1) % MaxJobs;
  count_++;
  return FIFOError::None;
}

void JobChannel::trait_pop() noexcept {
  std::unique_lock<std::mutex> lock(mtx_);
  if (!empty()) {
    head_ = (head_ + 1) % MaxJobs;
    count_--;
  }
}

void JobChannel::trait_clear() noexcept {
  std::unique_lock<std::mutex> lock(mtx_);
  head_ = 0;
  tail_ = 0;
  count_ = 0;
}

}  // namespace PawnDB
