#include "pawndb/types/job_channel.h"

namespace PawnDB {

job_channel::job_channel() noexcept : jobs_(), head_(0), tail_(0), count_(0) {}

job_channel::queue_r job_channel::get() noexcept {
  std::unique_lock<std::mutex> lock(mtx_);
  if (!empty()) {
    return jobs_[head_];
  } else {
    return QueueError::Empty;
  }
}

job_channel::queue_r job_channel::recv() noexcept {
  std::unique_lock<std::mutex> lock(mtx_);
  auto w_r =
      not_empty_.wait_for(lock, WAIT_TIMEOUT, [this] { return !empty(); });
  if (w_r) {
    return jobs_[head_];
  } else {
    return QueueError::Timeout;
  }
}

QueueError job_channel::send(const job& job) noexcept {
  std::unique_lock<std::mutex> lock(mtx_);
  if (full()) {
    return QueueError::Full;
  }
  jobs_[tail_] = job;
  tail_ = (tail_ + 1) % MaxJobs;
  count_++;
  return QueueError::None;
}

void job_channel::pop() noexcept {
  std::unique_lock<std::mutex> lock(mtx_);
  jobs_[head_].~job();
  head_ = (head_ + 1) % MaxJobs;
  count_--;
}

void job_channel::clear() noexcept {
  std::unique_lock<std::mutex> lock(mtx_);
  head_ = 0;
  tail_ = 0;
  count_ = 0;
}

void job_channel::notify_not_empty() noexcept {
  not_empty_.notify_one();
}

std::size_t job_channel::size() const noexcept {
  return count_;
}

bool job_channel::full() const noexcept {
  return count_ >= MaxJobs;
}

bool job_channel::empty() const noexcept {
  return count_ == 0;
}

}  // namespace PawnDB
