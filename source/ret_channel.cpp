#include "pawndb/types/ret_channel.h"

#include <cstddef>

#include "pawndb/traits/queue.h"

namespace PawnDB {

RetChannel::queue_r RetChannel::trait_get() noexcept {
  std::unique_lock<std::mutex> lock(mtx_);
  if (!trait_empty()) {
    return txns_[head_];
  } else {
    return QueueError::Empty;
  }
}

QueueError RetChannel::trait_send(const txn_id_t& _dead_txn) noexcept {
  std::unique_lock<std::mutex> lock(mtx_);
  txns_[tail_] = _dead_txn;
  tail_ = (tail_ + 1) % MAX_TRANSACTIONS;
  count_++;
  return QueueError::None;
}

void RetChannel::trait_clear() noexcept {
  std::unique_lock<std::mutex> lock(mtx_);
  head_ = 0;
  tail_ = 0;
  count_ = 0;
}

void RetChannel::trait_pop() noexcept {
  std::unique_lock<std::mutex> lock(mtx_);
  head_ = (head_ + 1) % MAX_TRANSACTIONS;
  count_--;
}

std::size_t RetChannel::trait_size() const noexcept {
  return count_;
}

bool RetChannel::trait_empty() const noexcept {
  return count_ == 0;
}

bool RetChannel::trait_full() const noexcept {
  return count_ == MAX_TRANSACTIONS;
}

}  // namespace PawnDB
