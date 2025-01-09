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
  // Sized
  std::size_t trait_size() const noexcept { return count_; }
  // Container
  std::size_t constexpr trait_capacity() const noexcept {
    return MAX_TRANSACTIONS;
  }
  bool trait_full() const noexcept { return count_ == MAX_TRANSACTIONS; }
  bool trait_empty() const noexcept { return count_ == 0; }
  // FIFO
  GetR trait_get() noexcept;
  GetR trait_recv() noexcept;
  FIFOError trait_send(const txn_id_t& _dead_txn) noexcept;
  FIFOError trait_send(txn_id_t&& _dead_txn) noexcept;
  void trait_clear() noexcept;
  void trait_pop() noexcept;
  void trait_notify_not_empty() noexcept;
};

}  // namespace PawnDB

#endif
