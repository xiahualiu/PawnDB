#include "pawndb/types/job_channel.h"

#include <sys/socket.h>

#include <cstring>

namespace PawnDB {

job::job(buf_ref _buffer, std::size_t _buffer_size, const sockaddr_un& _addr,
         socklen_t _addr_len) noexcept
    : buffer_(_buffer),
      buffer_size_(_buffer_size),
      client_addr_len_(_addr_len) {
  memcpy(&client_addr_, &_addr, sizeof(_addr));
}

job::job(const job& other) noexcept
    : buffer_(other.buffer_),
      buffer_size_(other.buffer_size_),
      client_addr_len_(other.client_addr_len_) {
  memcpy(&client_addr_, &other.client_addr_, sizeof(other.client_addr_));
}

job& job::operator=(const job& other) noexcept {
  buffer_ = other.buffer_;
  buffer_size_ = other.buffer_size_;
  client_addr_len_ = other.client_addr_len_;
  memcpy(&client_addr_, &other.client_addr_, sizeof(other.client_addr_));
  return *this;
}

buf_ref job::buf() const noexcept {
  return buffer_;
}

std::size_t job::buf_sz() const noexcept {
  return buffer_size_;
}

const sockaddr* job::c_addr() const noexcept {
  return reinterpret_cast<const sockaddr*>(&client_addr_);
}

socklen_t job::c_addr_len() const noexcept {
  return client_addr_len_;
}

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
