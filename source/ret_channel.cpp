#include "pawndb/types/ret_channel.h"

namespace PawnDB {

RetChannel::GetR RetChannel::trait_get() noexcept {
  std::unique_lock<std::mutex> lock(mtx_);
  if (!trait_empty()) {
    return txns_[head_];
  } else {
    return FIFOError::Empty;
  }
}

RetChannel::GetR RetChannel::trait_recv() noexcept { return trait_get(); }

FIFOError RetChannel::trait_send(const txn_id_t& _dead_txn) noexcept {
  std::unique_lock<std::mutex> lock(mtx_);
  txns_[tail_] = _dead_txn;
  tail_ = (tail_ + 1) % MAX_TRANSACTIONS;
  count_++;
  return FIFOError::None;
}

FIFOError RetChannel::trait_send(txn_id_t&& _dead_txn) noexcept {
  std::unique_lock<std::mutex> lock(mtx_);
  txns_[tail_] = std::move(_dead_txn);
  tail_ = (tail_ + 1) % MAX_TRANSACTIONS;
  count_++;
  return FIFOError::None;
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

void RetChannel::trait_notify_not_empty() noexcept {}

}  // namespace PawnDB
