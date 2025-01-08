#ifndef PAWNDB_TYPES_RETURN_CHANNEL_H
#define PAWNDB_TYPES_RETURN_CHANNEL_H

#include <sys/socket.h>

#include <array>
#include <cstddef>
#include <mutex>

#include "pawndb/params.h"
#include "pawndb/traits/container.h"
#include "pawndb/traits/fifo.h"
#include "pawndb/traits/sized.h"

namespace PawnDB {

class RetChannel : public FIFO<RetChannel, txn_id_t>,
                   public Sized<RetChannel>,
                   public Container<RetChannel> {
  std::array<txn_id_t, MAX_TRANSACTIONS> txns_;
  std::size_t head_ = 0;
  std::size_t tail_ = 0;
  std::size_t count_ = 0;
  std::mutex mtx_;

 public:
  std::size_t trait_size() const noexcept { return count_; }
  std::size_t constexpr trait_capacity() const noexcept {
    return MAX_TRANSACTIONS;
  }
  bool trait_full() const noexcept { return count_ == MAX_TRANSACTIONS; }
  bool trait_empty() const noexcept { return count_ == 0; }

  GetR trait_get() noexcept {
    std::unique_lock<std::mutex> lock(mtx_);
    if (!trait_empty()) {
      return txns_[head_];
    } else {
      return FIFOError::Empty;
    }
  }

  GetR trait_recv() noexcept { return trait_get(); }

  FIFOError trait_send(const txn_id_t& _dead_txn) noexcept {
    std::unique_lock<std::mutex> lock(mtx_);
    txns_[tail_] = _dead_txn;
    tail_ = (tail_ + 1) % MAX_TRANSACTIONS;
    count_++;
    return FIFOError::None;
  }

  FIFOError trait_send(txn_id_t&& _dead_txn) noexcept {
    std::unique_lock<std::mutex> lock(mtx_);
    txns_[tail_] = std::move(_dead_txn);
    tail_ = (tail_ + 1) % MAX_TRANSACTIONS;
    count_++;
    return FIFOError::None;
  }

  void trait_clear() noexcept {
    std::unique_lock<std::mutex> lock(mtx_);
    head_ = 0;
    tail_ = 0;
    count_ = 0;
  }

  void trait_pop() noexcept {
    std::unique_lock<std::mutex> lock(mtx_);
    head_ = (head_ + 1) % MAX_TRANSACTIONS;
    count_--;
  }

  void trait_notify_not_empty() noexcept {}
};

}  // namespace PawnDB

#endif
