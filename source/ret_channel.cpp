#include "pawndb/types/ret_channel.h"

#include <cstddef>

#include "pawndb/traits/channel.h"

namespace PawnDB {

ChannelError RetChannel::trait_send(const txn_id_t& _dead_txn) noexcept {
  std::unique_lock<std::mutex> lock(mtx_);
  txns_[tail_] = _dead_txn;
  tail_ = (tail_ + 1) % MAX_TRANSACTIONS;
  count_++;
  return ChannelError::None;
}

RetChannel::channel_r RetChannel::trait_recv() noexcept {
  std::unique_lock<std::mutex> lock(mtx_);
  txn_id_t txn = txns_[head_];
  head_ = (head_ + 1) % MAX_TRANSACTIONS;
  count_--;
  return txn;
}

std::size_t RetChannel::trait_size() const noexcept {
  return count_;
}

bool RetChannel::trait_full() const noexcept {
  return count_ == MAX_TRANSACTIONS;
}

bool RetChannel::trait_empty() const noexcept {
  return count_ == 0;
}

void RetChannel::trait_clear() noexcept {
  std::unique_lock<std::mutex> lock(mtx_);
  head_ = 0;
  tail_ = 0;
  count_ = 0;
}

}  // namespace PawnDB
