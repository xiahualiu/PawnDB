#include "pawndb/types/job_channel.h"

#include <sys/socket.h>

#include <cstring>

namespace PawnDB {

Job::Job(BufferRef _buffer, std::size_t _buffer_size, const sockaddr_un& _addr,
         socklen_t _addr_len) noexcept
    : buffer_(_buffer),
      buffer_size_(_buffer_size),
      client_addr_len_(_addr_len) {
  memcpy(&client_addr_, &_addr, sizeof(_addr));
}

Job::Job(const Job& other) noexcept
    : buffer_(other.buffer_),
      buffer_size_(other.buffer_size_),
      client_addr_len_(other.client_addr_len_) {
  memcpy(&client_addr_, &other.client_addr_, sizeof(other.client_addr_));
}

Job& Job::operator=(const Job& other) noexcept {
  buffer_ = other.buffer_;
  buffer_size_ = other.buffer_size_;
  client_addr_len_ = other.client_addr_len_;
  memcpy(&client_addr_, &other.client_addr_, sizeof(other.client_addr_));
  return *this;
}

Job Job::copy() const noexcept {
  return Job(*this);
}

void Job::copy_from(const Job& other) noexcept {
  buffer_ = other.buffer_;
  buffer_size_ = other.buffer_size_;
  client_addr_len_ = other.client_addr_len_;
  memcpy(&client_addr_, &other.client_addr_, sizeof(other.client_addr_));
}

BufferRef Job::buffer() const noexcept {
  return buffer_;
}

std::size_t Job::buffer_size() const noexcept {
  return buffer_size_;
}

const sockaddr* Job::c_addr() const noexcept {
  return reinterpret_cast<const sockaddr*>(&client_addr_);
}

socklen_t Job::c_addr_len() const noexcept {
  return client_addr_len_;
}

JobChannel::JobChannel() noexcept : jobs_(), head_(0), tail_(0), count_(0) {}

JobChannel::queue_r JobChannel::get() noexcept {
  std::unique_lock<std::mutex> lock(mtx_);
  if (!empty()) {
    return jobs_[head_];
  } else {
    return QueueError::Empty;
  }
}

JobChannel::queue_r JobChannel::recv() noexcept {
  std::unique_lock<std::mutex> lock(mtx_);
  auto w_r =
      not_empty_.wait_for(lock, WAIT_TIMEOUT, [this] { return !empty(); });
  if (w_r) {
    return jobs_[head_];
  } else {
    return QueueError::Timeout;
  }
}

QueueError JobChannel::send(const Job& job) noexcept {
  std::unique_lock<std::mutex> lock(mtx_);
  if (full()) {
    return QueueError::Full;
  }
  jobs_[tail_] = job;
  tail_ = (tail_ + 1) % MaxJobs;
  count_++;
  return QueueError::None;
}

void JobChannel::pop() noexcept {
  std::unique_lock<std::mutex> lock(mtx_);
  jobs_[head_].~Job();
  head_ = (head_ + 1) % MaxJobs;
  count_--;
}

void JobChannel::clear() noexcept {
  std::unique_lock<std::mutex> lock(mtx_);
  head_ = 0;
  tail_ = 0;
  count_ = 0;
}

void JobChannel::notify_not_empty() noexcept {
  not_empty_.notify_one();
}

std::size_t JobChannel::size() const noexcept {
  return count_;
}

bool JobChannel::full() const noexcept {
  return count_ >= MaxJobs;
}

bool JobChannel::empty() const noexcept {
  return count_ == 0;
}

}  // namespace PawnDB