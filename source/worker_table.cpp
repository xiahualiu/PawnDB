#include "pawndb/types/worker_table.h"

#include <sys/socket.h>
#include <sys/types.h>

#include <atomic>
#include <cstring>

#include "pawndb/traits/parser.h"
#include "pawndb/traits/table.h"
#include "pawndb/types/job_channel.h"
#include "pawndb/types/worker.h"

namespace PawnDB {

WorkerContext::WorkerContext() noexcept
    : ret_ch_(nullptr),
      db_(nullptr),
      job_ch_(),
      txn_id_(0),
      fd_(0),
      running_() {}

WorkerContext::WorkerContext(RetChannel* _ret_ch, Database* _db,
                             txn_id_t _txn_id, int _fd) noexcept
    : ret_ch_(_ret_ch),
      db_(_db),
      job_ch_(),
      txn_id_(_txn_id),
      fd_(_fd),
      running_() {}

WorkerContext::WorkerContext(const WorkerContext& _other) noexcept
    : ret_ch_(_other.ret_ch_),
      db_(_other.db_),
      job_ch_(),
      txn_id_(_other.txn_id_),
      fd_(_other.fd_) {}

WorkerContext& WorkerContext::operator=(const WorkerContext& _other) noexcept {
  ret_ch_ = _other.ret_ch_;
  db_ = _other.db_;
  txn_id_ = _other.txn_id_;
  fd_ = _other.fd_;
  return *this;
}

WorkerContext::~WorkerContext() noexcept {
  if (thread_.joinable()) {
    trait_stop();
    trait_join();
  }
}

std::size_t WorkerContext::trait_hash() const noexcept {
  return txn_id_;
}

void WorkerContext::trait_start() noexcept {
  running_.test_and_set(std::memory_order_relaxed);
  thread_ = std::thread([this]() {
    auto worker = Worker(this);
    worker.start();
  });
}

void WorkerContext::trait_stop() noexcept {
  running_.clear(std::memory_order_relaxed);
}

void WorkerContext::trait_join() noexcept {
  // Ensure the worker is dead.
  thread_.join();
  // Ack all unfinished jobs that worker has not acked
  while (!job_ch_.empty()) {
    auto job = job_ch_.recv().unwrap();
    auto parser = Parser(job.buf(), job.buf_size());
    reply(OpAck::DEAD_TXN, parser, 7, job);
  }
}

bool WorkerContext::trait_is_running() noexcept {
  if (!running_.test_and_set(std::memory_order_relaxed)) {
    running_.clear(std::memory_order_relaxed);
    return false;
  }
  return true;
}

WorkerContext::key_t WorkerContext::trait_txn_id() const noexcept {
  return txn_id_;
}

Database* WorkerContext::trait_db() const noexcept {
  return db_;
}

int WorkerContext::trait_fd() const noexcept {
  return fd_;
}

JobChannel& WorkerContext::trait_job_ch() noexcept {
  return job_ch_;
}

RetChannel& WorkerContext::trait_ret_ch() noexcept {
  return *ret_ch_;
}

std::atomic_flag& WorkerContext::trait_running() noexcept {
  return running_;
}

void WorkerContext::reply(OpAck _ack, Parser& _parser, std::size_t _size,
                          Job& _job) noexcept {
  _parser.set_ack(_ack);
  sendto(fd_, _parser.get_buffer().data(), _size, 0, _job.c_addr(),
         _job.c_addr_len());
  _job.buf().release();
}

void WorkerContext::worker_quit() noexcept {
  // Ack all unfinished jobs
  while (!job_ch_.empty()) {
    auto job = job_ch_.recv().unwrap();
    auto parser = Parser(job.buf(), job.buf_size());
    reply(OpAck::DEAD_TXN, parser, 7, job);
  }
}



WorkerTable::WorkerTable() noexcept : table_{}, size_(0) {}

WorkerTable::~WorkerTable() noexcept {
  trait_clear();
}

WorkerTable::table_r WorkerTable::trait_insert(
    const entry_t& _context) noexcept {
  if (trait_full()) return TableError::Full;
  auto idx = _context.txn_id_ % MAX_TRANSACTIONS;
  while (true) {
    if (!table_[idx].is_used_ || table_[idx].is_deleted_) {
      table_[idx].context_ = _context;
      table_[idx].is_used_ = true;
      table_[idx].is_deleted_ = false;
      size_++;
      table_[idx].context_.start();
      return table_[idx].context_;
    }
    idx = (idx + 1) % MAX_TRANSACTIONS;
  }
}

WorkerTable::table_r WorkerTable::trait_search(const key_t& _key) noexcept {
  auto idx = _key % MAX_TRANSACTIONS;
  auto start = idx;
  do {
    if (!table_[idx].is_used_) {
      return TableError::NotFound;
    }
    if (table_[idx].context_.txn_id_ == _key && !table_[idx].is_deleted_) {
      return table_[idx].context_;
    }
    idx = (idx + 1) % MAX_TRANSACTIONS;
  } while (idx != start);
  return TableError::NotFound;
}

TableError WorkerTable::trait_remove(const key_t& _key) noexcept {
  auto idx = _key % MAX_TRANSACTIONS;
  while (true) {
    if (table_[idx].context_.txn_id_ == _key && !table_[idx].is_deleted_) {
      table_[idx].context_.stop();
      table_[idx].context_.join();
      table_[idx].is_deleted_ = true;
      size_--;
      return TableError::None;
    }
    idx = (idx + 1) % MAX_TRANSACTIONS;
  }
}

/** @brief Clear all workers */
void WorkerTable::trait_clear() noexcept {
  for (auto& entry : table_) {
    if (entry.is_used_ && !entry.is_deleted_) {
      entry.context_.stop();
    }
  }
  for (auto& entry : table_) {
    if (entry.is_used_ && !entry.is_deleted_) {
      entry.context_.join();
      entry.is_deleted_ = true;
      size_--;
    }
  }
}

}  // namespace PawnDB
