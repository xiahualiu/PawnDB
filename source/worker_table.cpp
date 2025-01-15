#include "pawndb/types/worker_table.h"

#include <sys/socket.h>

#include <atomic>
#include <cstring>

#include "pawndb/traits/hash_table.h"
#include "pawndb/types/job_channel.h"
#include "pawndb/types/worker.h"

namespace PawnDB {

WorkerEntry::WorkerEntry() noexcept
    : ret_ch_(nullptr),
      db_(nullptr),
      txn_id_(0),
      fd_(0),
      running_(),
      is_used_(false),
      is_deleted_(false) {}

WorkerEntry::WorkerEntry(RetChannel* _ret_ch, Database* _db, txn_id_t _txn_id,
                         int _fd) noexcept
    : ret_ch_(_ret_ch),
      db_(_db),
      txn_id_(_txn_id),
      fd_(_fd),
      running_(),
      is_used_(false),
      is_deleted_(false) {}

WorkerEntry::WorkerEntry(const WorkerEntry& _other) noexcept
    : ret_ch_(_other.ret_ch_),
      db_(_other.db_),
      txn_id_(_other.txn_id_),
      fd_(_other.fd_) {}

WorkerEntry& WorkerEntry::operator=(const WorkerEntry& _other) noexcept {
  ret_ch_ = _other.ret_ch_;
  db_ = _other.db_;
  txn_id_ = _other.txn_id_;
  fd_ = _other.fd_;
  return *this;
}

std::size_t WorkerEntry::trait_hash() const noexcept {
  return txn_id_;
}

void WorkerEntry::trait_start() noexcept {
  running_.test_and_set(std::memory_order_relaxed);
  thread_ = std::thread([this]() {
    auto worker = Worker(this);
    worker.start();
  });
}

void WorkerEntry::trait_stop() noexcept {
  running_.clear(std::memory_order_relaxed);
}

void WorkerEntry::trait_join() noexcept {
  running_.clear(std::memory_order_relaxed);
  // Clear all unfinished jobs
  while (!job_ch_.empty()) {
    auto job = job_ch_.get().unwrap();
    auto parser = Parser(job.buffer(), job.buffer_size());
    parser.set_ack(OpAck::DEAD_TXN);
    sendto(fd_, parser.get_buffer().data(), 7, 0, &job.c_addr(),
           job.c_addr_len());
    job.buffer().release();
    job_ch_.pop();
  }
  thread_.join();
}

bool WorkerEntry::trait_is_running() noexcept {
  if (!running_.test_and_set(std::memory_order_relaxed)) {
    running_.clear(std::memory_order_relaxed);
    return false;
  }
  return true;
}

WorkerEntry WorkerEntry::trait_clone() const noexcept {
  return *this;
}

void WorkerEntry::trait_copy(const WorkerEntry& _other) noexcept {
  this->operator=(_other);
}

WorkerEntry::queue_r WorkerEntry::trait_get() noexcept {
    return job_ch_.get();
}

WorkerEntry::queue_r WorkerEntry::trait_recv() noexcept {
    return job_ch_.recv();
}

QueueError WorkerEntry::trait_send(const Job& job) noexcept {
    return job_ch_.send(job);
}

void WorkerEntry::trait_pop() noexcept {
    job_ch_.pop();
}

void WorkerEntry::trait_clear() noexcept {
    job_ch_.clear();
}

void WorkerEntry::trait_notify_not_empty() noexcept {
    job_ch_.notify_not_empty();
}

WorkerTable::table_r WorkerTable::trait_insert(
    const entry_type& _entry) noexcept {
  if (trait_full()) return TableError::Full;
  auto idx = _entry.txn_id_ % MAX_TRANSACTIONS;
  auto start = idx;
  do {
    if (!table_[idx].is_used_ || table_[idx].is_deleted_) {
      table_[idx] = _entry;
      table_[idx].is_used_ = true;
      table_[idx].is_deleted_ = false;
      size_++;
      return table_[idx];
    }
    idx = (idx + 1) % MAX_TRANSACTIONS;
  } while (idx != start);
  return TableError::Full;
}

WorkerTable::table_r WorkerTable::trait_search(const key_t& _key) noexcept {
  auto idx = _key % MAX_TRANSACTIONS;
  auto start = idx;
  do {
    if (!table_[idx].is_used_) {
      return TableError::NotFound;
    }
    if (table_[idx].txn_id_ == _key && !table_[idx].is_deleted_) {
      return table_[idx];
    }
    idx = (idx + 1) % MAX_TRANSACTIONS;
  } while (idx != start);
  return TableError::NotFound;
}

TableError WorkerTable::trait_remove(const key_t& _key) noexcept {
  auto idx = _key % MAX_TRANSACTIONS;
  auto start = idx;
  do {
    if (!table_[idx].is_used_) {
      return TableError::NotFound;
    }
    if (table_[idx].txn_id_ == _key && !table_[idx].is_deleted_) {
      table_[idx].is_deleted_ = true;
      size_--;
      return TableError::None;
    }
    idx = (idx + 1) % MAX_TRANSACTIONS;
  } while (idx != start);
  return TableError::NotFound;
}

TableError WorkerTable::trait_write(
    const WorkerTable::entry_type& _entry) noexcept {
  auto idx = _entry.txn_id_ % MAX_TRANSACTIONS;
  auto start = idx;
  do {
    if (!table_[idx].is_used_) return TableError::NotFound;
    if (table_[idx].txn_id_ == _entry.txn_id_ && !table_[idx].is_deleted_) {
      table_[idx] = _entry;
      return TableError::None;
    }
    idx = (idx + 1) % MAX_TRANSACTIONS;
  } while (idx != start);
  return TableError::NotFound;
}

}  // namespace PawnDB
