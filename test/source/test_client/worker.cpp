#include "pawndb/types/worker.h"
#include <sys/socket.h>
#include <sys/un.h>

#include <atomic>
#include <iostream>

#include "pawndb/params.h"
#include "pawndb/table_types.h"
#include "pawndb/traits/queue.h"
#include "pawndb/types/job_channel.h"

namespace PawnDB {

Worker::Worker(WorkerContext* entry) noexcept
    : running_(entry->running_),
      job_ch_(entry->job_ch_),
      ret_ch_(*entry->ret_ch_),
      db_(*entry->db_),
      txn_id_(entry->txn_id_),
      fd_(entry->fd_),
      timeout_cnt_(0),
      commit_table_(),
      status_(TxnStatus::GROWING),
      lock_table_() {}

void Worker::reply(OpAck _ack, Parser& _parser, std::size_t _size,
                   Job& _job) noexcept {
  _parser.set_ack(_ack);
  sendto(fd_, _parser.get_buffer().data(), _size, 0, _job.c_addr(),
         _job.c_addr_len());
}

bool Worker::trait_is_running() noexcept {
  if (!running_.test_and_set(std::memory_order_relaxed)) {
    running_.clear(std::memory_order_relaxed);
    return false;
  }
  return true;
}

void Worker::worker_quit() noexcept {
  release_locks();
  clear_commit_table();
  ret_ch_.send(txn_id_);
}

void Worker::release_locks() noexcept {
  auto lock_it = lock_table_.begin();
  auto lock_it_end = lock_table_.end();
  while (lock_it != lock_it_end) {
    auto& lock = *lock_it;
    auto [tbl, tp_key] = lock.key().disassemble();
    switch (tbl) {
      case tbl_id<StudentTable>(): {
        auto& table_ref = db_.students_;
        table_ref.release(tp_key);
        break;
      }
      default: {
        break;
      }
    }
    lock_it++;
  }
  lock_table_.clear();
}

void Worker::clear_commit_table() noexcept {
  while (!commit_table_.empty()) {
    auto commit = commit_table_.get().unwrap();
    commit.buffer().release();
    commit_table_.pop();
  }
}

void Worker::process_commit(Parser& _parser, Job& _job) noexcept {
  // Check if transaction is in the correct phase
  if (status_ != TxnStatus::SHRINKING) {
    reply(OpAck::SUCCESS, _parser, 7, _job);
    _job.buffer().release();
    job_ch_.pop();
    return;
  }
  // No error allowed in commit phase because of ACID properties
  while (!commit_table_.empty()) {
    auto& commit = commit_table_.get().unwrap();
    commit_table_.pop();
    switch (commit.op()) {
      case OpType::ADD_TUPLE: {
        auto [table_id, tuple_key] = commit.key().trait_disassemble();
        auto buffer = commit.buffer();
        switch (table_id) {
          case tbl_id<StudentTable>(): {
            auto new_tuple = StudentTable::tuple_t{};
            auto& table_ref = db_.students_;
            std::memcpy(&new_tuple, buffer.buffer().data(), sizeof(new_tuple));
            table_ref.insert(new_tuple);
            table_ref.notify_not_empty();
            table_ref.notify_shared();
            table_ref.notify_exclusive();
            commit.buffer().release();
            break;
          }
          default: {
            break;
          }
        }
        break;
      }
      case OpType::DELETE: {
        auto [table_id, tuple_key] = commit.key().trait_disassemble();
        switch (table_id) {
          case tbl_id<StudentTable>(): {
            auto& table_ref = db_.students_;
            table_ref.remove(tuple_key);
            table_ref.notify_not_full();
            break;
          }
          default: {
            break;
          }
        }
        break;
      }
      case OpType::UPDATE: {
        auto [table_id, tuple_key] = commit.key().trait_disassemble();
        auto buffer = commit.buffer();
        switch (table_id) {
          case tbl_id<StudentTable>(): {
            auto new_tuple = StudentTable::tuple_t{};
            auto& table_ref = db_.students_;
            std::memcpy(&new_tuple, buffer.buffer().data(), sizeof(new_tuple));
            table_ref.write(new_tuple);
            commit.buffer().release();
            break;
          }
          default: {
            break;
          }
        }
        break;
      }
      default: {
        break;
      }
    }
  }
  status_ = TxnStatus::COMMITTED;
  reply(OpAck::SUCCESS, _parser, _parser.get_buffer_size(), _job);
  _job.buffer().release();
  job_ch_.pop();
  return;
}

void Worker::process_add(Parser& _parser, Job& _job) noexcept {
  // Check if transaction is in the correct phase
  if (status_ == TxnStatus::GROWING) {
    status_ = TxnStatus::SHRINKING;
  }
  auto table_id_r = _parser.get_tbl();
  // Check if table ID is valid
  if (!table_id_r) {
    reply(OpAck::BAD_TABLE, _parser, 7, _job);
    _job.buffer().release();
    job_ch_.pop();
    return;
  }
  auto table_id = table_id_r.unwrap();
  switch (table_id) {
    case tbl_id<StudentTable>(): {
      auto new_entry = StudentTuple{};
      auto bytes_read_r =
          new_entry.deserialize(_job.buffer(), _parser.get_tuple_offset());
      // Check if tuple data is valid
      if (!bytes_read_r) {
        _parser.set_buffer_size(7);
        reply(OpAck::BAD_DATA, _parser, 7, _job);
        _job.buffer().release();
        job_ch_.pop();
        return;
      }
      // Reuse buffer
      std::memcpy(_job.buffer().buffer().data(), &new_entry, sizeof(new_entry));
      break;
    }
    default: {
      _parser.set_buffer_size(7);
      reply(OpAck::BAD_TABLE, _parser, 7, _job);
      _job.buffer().release();
      job_ch_.pop();
      return;
    }
  }
  // Insert new tuple into commit table
  auto commit_error =
      commit_table_.send({{table_id, 0}, OpType::ADD_TUPLE, _job.buffer()});
  // Check if commit table is full
  if (commit_error == QueueError::Full) {
    reply(OpAck::COMMIT_FULL, _parser, 7, _job);
    _job.buffer().release();
    job_ch_.pop();
    return;
  }
  reply(OpAck::SUCCESS, _parser, _parser.get_buffer_size(), _job);
  job_ch_.pop();
  // Don't release buffer here, it will be released when the transaction is
  // committed
  return;
}

void Worker::process_shared_read(Parser& _parser, Job& _job) noexcept {
  // Check if transaction is in the correct phase
  if (status_ != TxnStatus::GROWING) {
    reply(OpAck::BAD_PHASE, _parser, 7, _job);
    _job.buffer().release();
    job_ch_.pop();
    return;
  }
  auto table_id_r = _parser.get_tbl();
  // Check if table ID is valid
  if (!table_id_r) {
    reply(OpAck::BAD_TABLE, _parser, 7, _job);
    _job.buffer().release();
    job_ch_.pop();
    return;
  }
  auto table_id = table_id_r.unwrap();
  switch (table_id) {
    case tbl_id<StudentTable>(): {
      auto& table_ref = db_.students_;
      auto timeout_cnt = 0;
      while (true) {
        if (timeout_cnt >= MAX_TIMEOUT_RETRY) {
          reply(OpAck::TIMEOUT, _parser, 7, _job);
          _job.buffer().release();
          job_ch_.pop();
          return;
        }
        // Check if transaction is still running
        if (!is_running()) {
          reply(OpAck::ABORTED, _parser, 7, _job);
          status_ = TxnStatus::ABORTED;
          _job.buffer().release();
          job_ch_.pop();
          return;
        }
        auto wait_r = table_ref.wait_shared();
        if (!wait_r) {
          timeout_cnt++;
          continue;
        }
        auto add_lock_r = lock_table_.add_lock(
            {table_id, wait_r.unwrap().key()}, LockType::SHARED);
        // Check if lock is valid
        if (add_lock_r != LockError::None) {
          reply(OpAck::BAD_ACCESS, _parser, 7, _job);
          _job.buffer().release();
          job_ch_.pop();
        }
        auto student_entry = wait_r.unwrap();
        _parser.set_key(student_entry.key());
        auto offset_r =
            student_entry.serialize(_job.buffer(), _parser.get_tuple_offset());
        _parser.set_buffer_size(_parser.get_tuple_offset() + offset_r.unwrap());
        reply(OpAck::SUCCESS, _parser, _parser.get_buffer_size(), _job);
        _job.buffer().release();
        job_ch_.pop();
        return;
      }
    }
    default: {
      reply(OpAck::BAD_TABLE, _parser, 7, _job);
      _job.buffer().release();
      job_ch_.pop();
      return;
    }
  }
}

void Worker::process_exclusive_read(Parser& _parser, Job& _job) noexcept {
  // Check if transaction is in the correct phase
  if (status_ != TxnStatus::GROWING) {
    reply(OpAck::BAD_PHASE, _parser, 7, _job);
    _job.buffer().release();
    job_ch_.pop();
    return;
  }
  auto table_id_r = _parser.get_tbl();
  // Check if table ID is valid
  if (!table_id_r) {
    reply(OpAck::BAD_TABLE, _parser, 7, _job);
    _job.buffer().release();
    job_ch_.pop();
    return;
  }
  auto table_id = table_id_r.unwrap();
  switch (table_id) {
    case tbl_id<StudentTable>(): {
      auto& table_ref = db_.students_;
      auto timeout_cnt = 0;
      while (true) {
        if (timeout_cnt >= MAX_TIMEOUT_RETRY) {
          reply(OpAck::TIMEOUT, _parser, 7, _job);
          _job.buffer().release();
          job_ch_.pop();
          return;
        }
        // Check if transaction is still running
        if (!is_running()) {
          reply(OpAck::ABORTED, _parser, 7, _job);
          status_ = TxnStatus::ABORTED;
          _job.buffer().release();
          job_ch_.pop();
          return;
        }
        auto wait_r = table_ref.wait_exclusive();
        if (!wait_r) {
          timeout_cnt++;
          continue;
        }
        auto add_lock_r = lock_table_.add_lock(
            {table_id, wait_r.unwrap().key()}, LockType::EXCLUSIVE);
        // Check if lock is valid
        if (add_lock_r != LockError::None) {
          reply(OpAck::BAD_ACCESS, _parser, 7, _job);
          _job.buffer().release();
          job_ch_.pop();
          return;
        }
        auto student_entry = wait_r.unwrap();
        _parser.set_key(student_entry.key());
        auto offset_r =
            student_entry.serialize(_job.buffer(), _parser.get_tuple_offset());
        _parser.set_buffer_size(_parser.get_tuple_offset() + offset_r.unwrap());
        reply(OpAck::SUCCESS, _parser, _parser.get_buffer_size(), _job);
        _job.buffer().release();
        job_ch_.pop();
        return;
      }
    }
    default: {
      reply(OpAck::BAD_TABLE, _parser, 7, _job);
      _job.buffer().release();
      job_ch_.pop();
      return;
    }
  }
}

void Worker::process_yield(Parser& _parser, Job& _job) noexcept {
  if (status_ != TxnStatus::GROWING) {
    reply(OpAck::BAD_PHASE, _parser, 7, _job);
    _job.buffer().release();
    job_ch_.pop();
    return;
  }
  auto table_id_r = _parser.get_tbl();
  // Check if table ID is valid
  if (!table_id_r) {
    reply(OpAck::BAD_TABLE, _parser, 7, _job);
    _job.buffer().release();
    job_ch_.pop();
    return;
  }
  auto tuple_key_r = _parser.get_key();
  // Check if tuple key is valid
  if (!tuple_key_r) {
    reply(OpAck::BAD_TP, _parser, 7, _job);
    _job.buffer().release();
    job_ch_.pop();
    return;
  }
  auto table_id = table_id_r.unwrap();
  auto tuple_key = tuple_key_r.unwrap();
  auto lock_r = lock_table_.get_lock({table_id, tuple_key});
  // Check if lock is valid
  if (!lock_r || lock_r.unwrap() != LockType::SHARED) {
    reply(OpAck::BAD_ACCESS, _parser, 7, _job);
    _job.buffer().release();
    job_ch_.pop();
    return;
  }
  // because we checked the lock above, it is safe to ignore the return value
  lock_table_.rm_lock({table_id, tuple_key});
  // Release lock on entry
  switch (table_id) {
    case tbl_id<StudentTable>(): {
      auto& table_ref = db_.students_;
      table_ref.release(tuple_key);
      reply(OpAck::SUCCESS, _parser, _parser.get_buffer_size(), _job);
      _job.buffer().release();
      job_ch_.pop();
      return;
    }
    default: {
      reply(OpAck::BAD_TABLE, _parser, 7, _job);
      _job.buffer().release();
      job_ch_.pop();
      return;
    }
  }
}

void Worker::process_promote(Parser& _parser, Job& _job) noexcept {
  // Check if transaction is in the correct phase
  if (status_ != TxnStatus::GROWING) {
    reply(OpAck::BAD_PHASE, _parser, 7, _job);
    _job.buffer().release();
    job_ch_.pop();
    return;
  }
  auto table_id_r = _parser.get_tbl();
  // Check if table ID is valid
  if (!table_id_r) {
    reply(OpAck::BAD_TABLE, _parser, 7, _job);
    _job.buffer().release();
    job_ch_.pop();
    return;
  }
  auto tuple_key_r = _parser.get_key();
  // Check if tuple key is valid
  if (!tuple_key_r) {
    reply(OpAck::BAD_TP, _parser, 7, _job);
    _job.buffer().release();
    job_ch_.pop();
    return;
  }
  auto table_id = table_id_r.unwrap();
  auto tuple_key = tuple_key_r.unwrap();
  auto lock_r = lock_table_.get_lock({table_id, tuple_key});
  // Check if lock is valid
  if (!lock_r || lock_r.unwrap() != LockType::SHARED) {
    reply(OpAck::BAD_ACCESS, _parser, 7, _job);
    _job.buffer().release();
    job_ch_.pop();
    return;
  }
  switch (table_id) {
    case tbl_id<StudentTable>(): {
      auto& table_ref = db_.students_;
      auto timeout_cnt = 0;
      while (true) {
        // If the operation times out, abort the operation
        if (timeout_cnt >= MAX_TIMEOUT_RETRY) {
          reply(OpAck::TIMEOUT, _parser, 7, _job);
          status_ = TxnStatus::ABORTED;
          _job.buffer().release();
          job_ch_.pop();
          return;
        }
        // If the transaction is no longer running, abort the operation
        if (!is_running()) {
          reply(OpAck::ABORTED, _parser, 7, _job);
          _job.buffer().release();
          job_ch_.pop();
          return;
        }
        auto wait_r = table_ref.promote(tuple_key);
        if (wait_r != TupleTableError::None) {
          timeout_cnt++;
          continue;
        }
        break;
      }
      // Promote the lock in the lock table
      lock_table_.promote_lock({table_id, tuple_key});
      reply(OpAck::SUCCESS, _parser, _parser.get_buffer_size(), _job);
      _job.buffer().release();
      job_ch_.pop();
      return;
    }
    default: {
      reply(OpAck::BAD_TABLE, _parser, 7, _job);
      _job.buffer().release();
      job_ch_.pop();
      return;
    }
  }
}

void Worker::process_update(Parser& _parser, Job& _job) noexcept {
  if (status_ == TxnStatus::GROWING) {
    status_ = TxnStatus::SHRINKING;
  }
  auto table_id_r = _parser.get_tbl();
  // Check if table ID is valid
  if (!table_id_r) {
    reply(OpAck::BAD_TABLE, _parser, 7, _job);
    _job.buffer().release();
    job_ch_.pop();
    return;
  }
  auto tuple_key_r = _parser.get_key();
  // Check if tuple key is valid
  if (!tuple_key_r) {
    reply(OpAck::BAD_TP, _parser, 7, _job);
    _job.buffer().release();
    job_ch_.pop();
    return;
  }
  auto table_id = table_id_r.unwrap();
  auto tuple_key = tuple_key_r.unwrap();
  auto lock_r = lock_table_.get_lock({table_id, tuple_key});
  // Check if lock is valid
  if (!lock_r || lock_r.unwrap() != LockType::EXCLUSIVE) {
    reply(OpAck::BAD_ACCESS, _parser, 7, _job);
    _job.buffer().release();
    job_ch_.pop();
    return;
  }
  switch (table_id) {
    case tbl_id<StudentTable>(): {
      auto new_entry = StudentTuple{};
      if (!new_entry.deserialize(_job.buffer(), _parser.get_tuple_offset())) {
        reply(OpAck::BAD_DATA, _parser, 7, _job);
        _job.buffer().release();
        job_ch_.pop();
        return;
      }
      std::memcpy(_job.buffer().buffer().data(), &new_entry, sizeof(new_entry));
      break;
    }
    default: {
      reply(OpAck::BAD_TABLE, _parser, 7, _job);
      _job.buffer().release();
      job_ch_.pop();
      return;
    }
  }
  auto commit_error = commit_table_.send(
      {{table_id, tuple_key}, OpType::UPDATE, _job.buffer()});
  if (commit_error == QueueError::Full) {
    reply(OpAck::COMMIT_FULL, _parser, 7, _job);
    _job.buffer().release();
    job_ch_.pop();
    return;
  }
  reply(OpAck::SUCCESS, _parser, 7, _job);
  job_ch_.pop();
  // Don't release buffer here, it will be released when the transaction is
  // committed
  return;
}

void Worker::process_rm(Parser& _parser, Job& _job) noexcept {
  // Check if transaction is in the correct phase
  if (status_ == TxnStatus::GROWING) {
    status_ = TxnStatus::SHRINKING;
  }
  auto table_id_r = _parser.get_tbl();
  // Check if table ID is valid
  if (!table_id_r) {
    reply(OpAck::BAD_TABLE, _parser, 7, _job);
    _job.buffer().release();
    job_ch_.pop();
    return;
  }
  auto tuple_key_r = _parser.get_key();
  // Check if tuple key is valid
  if (!tuple_key_r) {
    reply(OpAck::BAD_TP, _parser, 7, _job);
    _job.buffer().release();
    job_ch_.pop();
    return;
  }
  auto table_id = table_id_r.unwrap();
  auto tuple_key = tuple_key_r.unwrap();
  auto lock_record_key = TableTupleKey{table_id, tuple_key};
  auto lock_r = lock_table_.get_lock(lock_record_key);
  // Check if lock is valid
  if (!lock_r || lock_r.unwrap() != LockType::EXCLUSIVE) {
    reply(OpAck::BAD_ACCESS, _parser, 7, _job);
    _job.buffer().release();
    job_ch_.pop();
    return;
  }
  auto commit_error = commit_table_.send({lock_record_key, OpType::DELETE, {}});
  // Check if commit table is full
  if (commit_error == QueueError::Full) {
    reply(OpAck::COMMIT_FULL, _parser, 7, _job);
    _job.buffer().release();
    job_ch_.pop();
    return;
  }
  reply(OpAck::SUCCESS, _parser, _parser.get_buffer_size(), _job);
  _job.buffer().release();
  job_ch_.pop();
  return;
}

void Worker::trait_start() noexcept {
  while (is_running()) {
    std::cout << "Worker running: #" << txn_id_ << std::endl;
    auto job_r = job_ch_.recv();
    if (!job_r) {
      timeout_cnt_++;
      continue;
    } else {
      timeout_cnt_ = 0;
    }
    if (timeout_cnt_ >= MAX_TIMEOUT_RETRY) {
      worker_quit();
      return;
    }
    auto job = job_r.unwrap();

    // DEBUG
    std::cout << "Worker #" << txn_id_ << " , get a job from: " << reinterpret_cast<const sockaddr_un*>(job.c_addr())->sun_path << std::endl;
    std::cout << "Length of client address: " << job.c_addr_len() << std::endl;

    auto parser = Parser(job.buffer(), job.buffer_size());
    auto op_r = parser.get_op();
    if (!op_r) {
      reply(OpAck::BAD_OP, parser, 7, job);
      job.buffer().release();
      job_ch_.pop();
      continue;
    }
    auto op = op_r.unwrap();
    switch (op) {
      case OpType::START_TXN:
        parser.set_txn_id(txn_id_);
        std::cout << "Worker #" << txn_id_ << " reply START_TXN." << std::endl;
        reply(OpAck::SUCCESS, parser, 7, job);
        job.buffer().release();
        job_ch_.pop();
        break;
      case OpType::COMMIT_TXN: {
        process_commit(parser, job);
        worker_quit();
        return;
      }
      case OpType::ABORT_TXN: {
        worker_quit();
        return;
      }
      case OpType::ADD_TUPLE: {
        process_add(parser, job);
        break;
      }
      case OpType::SHARED_READ: {
        process_shared_read(parser, job);
        break;
      }
      case OpType::EXCLUSIVE_READ: {
        process_exclusive_read(parser, job);
        break;
      }
      case OpType::YIELD_READ: {
        process_yield(parser, job);
        break;
      }
      case OpType::PROMOTE: {
        process_promote(parser, job);
        break;
      }
      case OpType::UPDATE: {
        process_update(parser, job);
        break;
      }
      case OpType::DELETE: {
        process_rm(parser, job);
        break;
      }
      default: {
        reply(OpAck::BAD_OP, parser, 7, job);
        job_ch_.pop();
        continue;
      }
    }
  }
  worker_quit();
  return;
}

}  // namespace PawnDB
