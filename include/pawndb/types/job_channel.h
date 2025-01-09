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
  // FIFO
  GetR trait_get() noexcept;
  GetR trait_recv() noexcept;
  FIFOError trait_send(const Job& job) noexcept;
  FIFOError trait_send(Job&& job) noexcept;
  void trait_pop() noexcept;
  void trait_clear() noexcept;
  void trait_notify_not_empty() noexcept { not_empty_.notify_one(); }
  // Sized
  std::size_t trait_size() const noexcept { return count_; }
  // Container
  std::size_t constexpr trait_capacity() const noexcept { return MaxJobs; }
  bool trait_full() const noexcept { return count_ == MaxJobs; }
  bool trait_empty() const noexcept { return count_ == 0; }
};

}  // namespace PawnDB

#endif
