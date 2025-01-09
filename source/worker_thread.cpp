/**
 * @file worker_thread.cpp
 * @brief Worker thread implementation for PawnDB
 * @version 0.1
 * @date 2025-01-02
 *
 * Handles:
 * - Transaction processing
 * - Lock management
 * - Request handling
 * - Client communication
 *
 * @copyright MIT License
 */

#include "pawndb/types/worker_thread.h"

#include <sys/types.h>

#include <atomic>
#include <cstdlib>
#include <cstring>

#include "pawndb/params.h"
#include "pawndb/table_types.h"
#include "pawndb/traits/lock_manager.h"
#include "pawndb/traits/parser.h"
#include "pawndb/types/commit_table.h"
#include "pawndb/types/job_channel.h"
#include "pawndb/types/lock_records.h"
#include "pawndb/types/parser.h"
#include "pawndb/types/student_table.h"
#include "pawndb/types/student_tuple.h"
#include "pawndb/types/table_tuple_key.h"

namespace PawnDB {

void WorkerThread::job_ack(const OpAck _ack, const WorkerContext& _ct,
                           Parser& _parser, Job& _job) noexcept {
  _parser.set_ack(_ack);
  sendto(_ct.server_fd, _parser.get_buffer(), _parser.get_buffer_size(), 0,
         &_job.client_addr_, _job.client_addr_len_);
  _ct.db->buffers.release(_job.buffer_index_);
  _ct.job_ch->pop();
}

void WorkerThread::process_add(const WorkerContext& _ct, WorkerRuntime& _rt,
                               Parser& _parser, Job& _job) noexcept {
  if (_rt.status == TxnStatus::GROWING) {
    _rt.status = TxnStatus::SHRINKING;
  }
  if (_rt.commit_table.full()) {
    _parser.set_buffer_size(7);
    job_ack(OpAck::COMMIT_FULL, _ct, _parser, _job);
    _ct.db->buffers.release(_job.buffer_index_);
    return;
  }
  auto table_id_r = _parser.get_tbl();
  if (!table_id_r) {
    _parser.set_buffer_size(7);
    job_ack(OpAck::BAD_TABLE, _ct, _parser, _job);
    _ct.db->buffers.release(_job.buffer_index_);
    return;
  }
  auto table_id = table_id_r.unwrap();
  auto new_buffer_index_r = _ct.db->buffers.request();
  if (!new_buffer_index_r) {
    _parser.set_buffer_size(7);
    job_ack(OpAck::BUSY, _ct, _parser, _job);
    _ct.db->buffers.release(_job.buffer_index_);
    return;
  }
  auto new_buffer_index = new_buffer_index_r.unwrap();
  auto& new_buffer_ref = _ct.db->buffers[new_buffer_index];
  auto job_buffer = _ct.db->buffers[_job.buffer_index_];
  auto tuple_key = tbl_row_t(0);
  switch (table_id) {
    case tbl_id<StudentTable>(): {
      auto new_tuple = StudentTuple{};
      auto bytes_read_r =
          new_tuple.deserialize(job_buffer.data(), _parser.get_tuple_offset());
      if (!bytes_read_r) {
        _parser.set_buffer_size(7);
        job_ack(OpAck::BAD_DATA, _ct, _parser, _job);
        _ct.db->buffers.release(_job.buffer_index_);
        return;
      }
      auto new_entry = StudentTableEntry{new_tuple, 0, 0, false, false};
      std::memcpy(new_buffer_ref.data(), &new_entry, sizeof(new_entry));
      break;
    }
    default: {
      _parser.set_buffer_size(7);
      job_ack(OpAck::BAD_TABLE, _ct, _parser, _job);
      _ct.db->buffers.release(_job.buffer_index_);
      return;
    }
  }
  _rt.commit_table.insert({{table_id, tuple_key},
                           OpType::ADD_TUPLE,
                           new_buffer_index,
                           false,
                           false});
  job_ack(OpAck::SUCCESS, _ct, _parser, _job);
  _ct.db->buffers.release(_job.buffer_index_);
  return;
}

void WorkerThread::process_rm(const WorkerContext& _ct, WorkerRuntime& _rt,
                              Parser& _parser, Job& _job) noexcept {
  if (_rt.status == TxnStatus::GROWING) {
    _rt.status = TxnStatus::SHRINKING;
  }
  if (_rt.commit_table.full()) {
    _parser.set_buffer_size(7);
    job_ack(OpAck::COMMIT_FULL, _ct, _parser, _job);
    _ct.db->buffers.release(_job.buffer_index_);
    return;
  }
  auto table_id_r = _parser.get_tbl();
  if (!table_id_r) {
    _parser.set_buffer_size(7);
    job_ack(OpAck::BAD_TABLE, _ct, _parser, _job);
    _ct.db->buffers.release(_job.buffer_index_);
    return;
  }
  auto tuple_key_r = _parser.get_key();
  if (!tuple_key_r) {
    _parser.set_buffer_size(7);
    job_ack(OpAck::BAD_TP, _ct, _parser, _job);
    _ct.db->buffers.release(_job.buffer_index_);
    return;
  }
  auto table_id = table_id_r.unwrap();
  auto tuple_key = tuple_key_r.unwrap();
  auto lock_record_key = TableTupleKey{table_id, tuple_key};
  auto lock_r = _rt.lock_table.get_lock(lock_record_key);
  if (!lock_r || lock_r.unwrap().type != LockType::Exclusive) {
    _parser.set_buffer_size(7);
    job_ack(OpAck::BAD_ACCESS, _ct, _parser, _job);
    _ct.db->buffers.release(_job.buffer_index_);
    return;
  }
  _rt.commit_table.insert({lock_record_key, OpType::DELETE, 0, 0, false});
  job_ack(OpAck::SUCCESS, _ct, _parser, _job);
  _ct.db->buffers.release(_job.buffer_index_);
  return;
}

void WorkerThread::process_shared_read(const WorkerContext& _ct,
                                       WorkerRuntime& _rt, Parser& _parser,
                                       Job& _job) noexcept {
  if (_rt.status != TxnStatus::GROWING) {
    _parser.set_buffer_size(7);
    job_ack(OpAck::BAD_PHASE, _ct, _parser, _job);
    _ct.db->buffers.release(_job.buffer_index_);
    return;
  }
  auto table_id_r = _parser.get_tbl();
  if (!table_id_r) {
    _parser.set_buffer_size(7);
    job_ack(OpAck::BAD_TABLE, _ct, _parser, _job);
    _ct.db->buffers.release(_job.buffer_index_);
    return;
  }
  auto table_id = table_id_r.unwrap();
  switch (table_id) {
    case tbl_id<StudentTable>(): {
      auto& table_ref = _ct.db->students;
      auto timeout_cnt = 0;
      while (true) {
        if (timeout_cnt >= MAX_TIMEOUT_RETRY) {
          job_ack(OpAck::TIMEOUT, _ct, _parser, _job);
          _ct.db->buffers.release(_job.buffer_index_);
          return;
        }
        if (!_ct.running->test_and_set(std::memory_order_acquire)) {
          _ct.running->clear(std::memory_order_release);
          job_ack(OpAck::ABORTED, _ct, _parser, _job);
          _ct.db->buffers.release(_job.buffer_index_);
          return;
        }
        auto wait_r = table_ref.wait_shared();
        if (!wait_r) {
          timeout_cnt++;
          continue;
        }
        if (LockError::LockConflict ==
            _rt.lock_table.lock({table_id, wait_r.unwrap().key},
                                LockType::Shared)) {
          _parser.set_buffer_size(7);
          job_ack(OpAck::BAD_ACCESS, _ct, _parser, _job);
          _ct.db->buffers.release(_job.buffer_index_);
        }
        auto student_entry = wait_r.unwrap();
        _parser.set_key(student_entry.key);
        auto offset = student_entry.tuple.serialize(_parser.get_buffer(),
                                                    _parser.get_tuple_offset());
        _parser.set_buffer_size(_parser.get_tuple_offset() + offset);
        job_ack(OpAck::SUCCESS, _ct, _parser, _job);
        _ct.db->buffers.release(_job.buffer_index_);
        return;
      }
    }
    default: {
      _parser.set_buffer_size(7);
      job_ack(OpAck::BAD_TABLE, _ct, _parser, _job);
      _ct.db->buffers.release(_job.buffer_index_);
      return;
    }
  }
}

void WorkerThread::process_exclusive_read(const WorkerContext& _ct,
                                          WorkerRuntime& _rt, Parser& _parser,
                                          Job& _job) noexcept {
  if (_rt.status != TxnStatus::GROWING) {
    _parser.set_buffer_size(7);
    job_ack(OpAck::BAD_PHASE, _ct, _parser, _job);
    _ct.db->buffers.release(_job.buffer_index_);
    return;
  }
  auto table_id_r = _parser.get_tbl();
  if (!table_id_r) {
    _parser.set_buffer_size(7);
    job_ack(OpAck::BAD_TABLE, _ct, _parser, _job);
    _ct.db->buffers.release(_job.buffer_index_);
    return;
  }
  auto table_id = table_id_r.unwrap();
  switch (table_id) {
    case tbl_id<StudentTable>(): {
      auto& table_ref = _ct.db->students;
      std::uint8_t timeout_cnt = 0;
      while (true) {
        if (timeout_cnt >= MAX_TIMEOUT_RETRY) {
          job_ack(OpAck::TIMEOUT, _ct, _parser, _job);
          _ct.db->buffers.release(_job.buffer_index_);
          return;
        }
        if (!_ct.running->test_and_set(std::memory_order_acquire)) {
          _ct.running->clear(std::memory_order_release);
          job_ack(OpAck::ABORTED, _ct, _parser, _job);
          _ct.db->buffers.release(_job.buffer_index_);
          return;
        }
        auto wait_r = table_ref.wait_exclusive();
        if (!wait_r) {
          timeout_cnt++;
          continue;
        }
        if (LockError::LockConflict ==
            _rt.lock_table.lock({table_id, wait_r.unwrap().key},
                                LockType::Exclusive)) {
          _parser.set_buffer_size(7);
          job_ack(OpAck::BAD_ACCESS, _ct, _parser, _job);
          _ct.db->buffers.release(_job.buffer_index_);
        }
        auto student_entry = wait_r.unwrap();
        _parser.set_key(student_entry.key);
        auto offset = student_entry.tuple.serialize(_parser.get_buffer(),
                                                    _parser.get_tuple_offset());
        _parser.set_buffer_size(_parser.get_tuple_offset() + offset);
        job_ack(OpAck::SUCCESS, _ct, _parser, _job);
        _ct.db->buffers.release(_job.buffer_index_);
        return;
      }
    }
    default: {
      _parser.set_buffer_size(7);
      job_ack(OpAck::BAD_TABLE, _ct, _parser, _job);
      _ct.db->buffers.release(_job.buffer_index_);
      return;
    }
  }
}

void WorkerThread::process_yield(const WorkerContext& _ct, WorkerRuntime& _rt,
                                 Parser& _parser, Job& _job) noexcept {
  if (_rt.status != TxnStatus::GROWING) {
    _parser.set_buffer_size(7);
    job_ack(OpAck::BAD_PHASE, _ct, _parser, _job);
    _ct.db->buffers.release(_job.buffer_index_);
    return;
  }
  auto table_id_r = _parser.get_tbl();
  if (!table_id_r) {
    _parser.set_buffer_size(7);
    job_ack(OpAck::BAD_TABLE, _ct, _parser, _job);
    _ct.db->buffers.release(_job.buffer_index_);
    return;
  }
  auto tuple_key_r = _parser.get_key();
  if (!tuple_key_r) {
    _parser.set_buffer_size(7);
    job_ack(OpAck::BAD_TP, _ct, _parser, _job);
    _ct.db->buffers.release(_job.buffer_index_);
    return;
  }
  auto table_id = table_id_r.unwrap();
  auto tuple_key = tuple_key_r.unwrap();
  auto lock_r = _rt.lock_table.get_lock({table_id, tuple_key});
  if (!lock_r || lock_r.unwrap().type != LockType::Shared) {
    _parser.set_buffer_size(7);
    job_ack(OpAck::BAD_ACCESS, _ct, _parser, _job);
    _ct.db->buffers.release(_job.buffer_index_);
    return;
  }
  _rt.lock_table.unlock({table_id, tuple_key});
  switch (table_id) {
    case tbl_id<StudentTable>(): {
      auto& table_ref = _ct.db->students;
      table_ref.release(tuple_key);
      job_ack(OpAck::SUCCESS, _ct, _parser, _job);
      _ct.db->buffers.release(_job.buffer_index_);
      return;
    }
    default: {
      _parser.set_buffer_size(7);
      job_ack(OpAck::BAD_TABLE, _ct, _parser, _job);
      _ct.db->buffers.release(_job.buffer_index_);
      return;
    }
  }
}

void WorkerThread::process_update(const WorkerContext& _ct, WorkerRuntime& _rt,
                                  Parser& _parser, Job& _job) noexcept {
  if (_rt.status == TxnStatus::GROWING) {
    _rt.status = TxnStatus::SHRINKING;
  }
  if (_rt.commit_table.full()) {
    _parser.set_buffer_size(7);
    job_ack(OpAck::COMMIT_FULL, _ct, _parser, _job);
    _ct.db->buffers.release(_job.buffer_index_);
    return;
  }
  auto table_id_r = _parser.get_tbl();
  if (!table_id_r) {
    _parser.set_buffer_size(7);
    job_ack(OpAck::BAD_TABLE, _ct, _parser, _job);
    _ct.db->buffers.release(_job.buffer_index_);
    return;
  }
  auto tuple_key_r = _parser.get_key();
  if (!tuple_key_r) {
    _parser.set_buffer_size(7);
    job_ack(OpAck::BAD_TP, _ct, _parser, _job);
    _ct.db->buffers.release(_job.buffer_index_);
    return;
  }
  auto table_id = table_id_r.unwrap();
  auto tuple_key = tuple_key_r.unwrap();
  auto lock_r = _rt.lock_table.get_lock({table_id, tuple_key});
  if (!lock_r || lock_r.unwrap().type != LockType::Exclusive) {
    _parser.set_buffer_size(7);
    job_ack(OpAck::BAD_ACCESS, _ct, _parser, _job);
    _ct.db->buffers.release(_job.buffer_index_);
    return;
  }
  auto new_buffer_index_r = _ct.db->buffers.request();
  if (!new_buffer_index_r) {
    _parser.set_buffer_size(7);
    job_ack(OpAck::BUSY, _ct, _parser, _job);
    _ct.db->buffers.release(_job.buffer_index_);
    return;
  }
  switch (table_id) {
    case tbl_id<StudentTable>(): {
      auto new_tuple = StudentTuple{};
      if (!new_tuple.deserialize(_ct.db->buffers[_job.buffer_index_].data(),
                                 _parser.get_tuple_offset())) {
        _parser.set_buffer_size(7);
        job_ack(OpAck::BAD_DATA, _ct, _parser, _job);
        _ct.db->buffers.release(_job.buffer_index_);
        return;
      }
      std::memcpy(_ct.db->buffers[new_buffer_index_r.unwrap()].data(),
                  &new_tuple, sizeof(new_tuple));
      break;
    }
    default: {
      _parser.set_buffer_size(7);
      job_ack(OpAck::BAD_TABLE, _ct, _parser, _job);
      _ct.db->buffers.release(_job.buffer_index_);
      return;
    }
  }
  _rt.commit_table.insert({{table_id, tuple_key},
                           OpType::UPDATE,
                           new_buffer_index_r.unwrap(),
                           false,
                           false});
  job_ack(OpAck::SUCCESS, _ct, _parser, _job);
  _ct.db->buffers.release(_job.buffer_index_);
  return;
}

/**
 * @brief Processes a lock promotion request
 *
 * Upgrades a shared lock to an exclusive lock in growing phase:
 * - Validates transaction is in growing phase
 * - Checks table ID and tuple key validity
 * - Verifies shared lock ownership
 * - Attempts lock promotion
 * - Updates lock table state
 *
 * Error cases:
 * - BAD_PHASE: Not in growing phase
 * - BAD_TABLE: Invalid table ID
 * - BAD_TP: Invalid tuple key
 * - BAD_ACCESS: Lock not held or wrong type
 * - LOCK_CONFLICT: Promotion failed
 *
 * @param _ct Worker execution context
 * @param _rt Worker runtime state
 * @param _parser Request parser
 * @param _job Current job details
 */
void WorkerThread::process_promote(const WorkerContext& _ct, WorkerRuntime& _rt,
                                   Parser& _parser, Job& _job) noexcept {
  if (_rt.status != TxnStatus::GROWING) {
    _parser.set_buffer_size(7);
    job_ack(OpAck::BAD_PHASE, _ct, _parser, _job);
    _ct.db->buffers.release(_job.buffer_index_);
    return;
  }
  auto table_id_r = _parser.get_tbl();
  if (!table_id_r) {
    _parser.set_buffer_size(7);
    job_ack(OpAck::BAD_TABLE, _ct, _parser, _job);
    _ct.db->buffers.release(_job.buffer_index_);
    return;
  }
  auto tuple_key_r = _parser.get_key();
  if (!tuple_key_r) {
    _parser.set_buffer_size(7);
    job_ack(OpAck::BAD_TP, _ct, _parser, _job);
    _ct.db->buffers.release(_job.buffer_index_);
    return;
  }
  auto table_id = table_id_r.unwrap();
  auto tuple_key = tuple_key_r.unwrap();
  auto lock_r = _rt.lock_table.get_lock({table_id, tuple_key});
  if (!lock_r || lock_r.unwrap().type != LockType::Shared) {
    _parser.set_buffer_size(7);
    job_ack(OpAck::BAD_ACCESS, _ct, _parser, _job);
    _ct.db->buffers.release(_job.buffer_index_);
    return;
  }
  switch (table_id) {
    case tbl_id<StudentTable>(): {
      auto& table_ref = _ct.db->students;
      auto timeout_cnt = 0;
      while (true) {
        if (timeout_cnt >= MAX_TIMEOUT_RETRY) {
          job_ack(OpAck::TIMEOUT, _ct, _parser, _job);
          _ct.db->buffers.release(_job.buffer_index_);
          return;
        }
        if (!_ct.running->test_and_set(std::memory_order_acquire)) {
          _ct.running->clear(std::memory_order_release);
          job_ack(OpAck::ABORTED, _ct, _parser, _job);
          _ct.db->buffers.release(_job.buffer_index_);
          return;
        }
        auto wait_r = table_ref.promote(tuple_key);
        if (wait_r != TupleTableError::None) {
          timeout_cnt++;
          continue;
        }
        break;
      }
      if (_rt.lock_table.promote({table_id, tuple_key}) != LockError::None) {
        _parser.set_buffer_size(7);
        job_ack(OpAck::BAD_ACCESS, _ct, _parser, _job);
        _ct.db->buffers.release(_job.buffer_index_);
        return;
      }
      job_ack(OpAck::SUCCESS, _ct, _parser, _job);
      _ct.db->buffers.release(_job.buffer_index_);
      return;
    }
    default: {
      _parser.set_buffer_size(7);
      job_ack(OpAck::BAD_TABLE, _ct, _parser, _job);
      _ct.db->buffers.release(_job.buffer_index_);
      return;
    }
  }
}

void WorkerThread::process_commit(const WorkerContext& _ct, WorkerRuntime& _rt,
                                  Parser& _parser, Job& _job) noexcept {
  if (_rt.status != TxnStatus::SHRINKING) {
    job_ack(OpAck::SUCCESS, _ct, _parser, _job);
    _ct.db->buffers.release(_job.buffer_index_);
    return;
  }
  auto commit_it = _rt.commit_table.begin();
  auto commit_it_end = _rt.commit_table.end();
  while (commit_it != commit_it_end) {
    auto& commit = *commit_it;
    switch (commit.op) {
      case OpType::ADD_TUPLE: {
        auto [table_id, tuple_key] = commit.key.trait_disassemble();
        auto& buffer = _ct.db->buffers[commit.buffer_index];
        switch (table_id) {
          case tbl_id<StudentTable>(): {
            auto new_tuple = StudentTable::entry_type{};
            auto& table_ref = _ct.db->students;
            std::memcpy(&new_tuple, buffer.data(), sizeof(new_tuple));
            table_ref.insert(new_tuple);
            table_ref.notify_not_empty();
            table_ref.notify_s();
            table_ref.notify_x();
            break;
          }
          default: {
            break;
          }
        }
        break;
      }
      case OpType::DELETE: {
        auto [table_id, tuple_key] = commit.key.trait_disassemble();
        switch (table_id) {
          case tbl_id<StudentTable>(): {
            auto& table_ref = _ct.db->students;
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
        auto [table_id, tuple_key] = commit.key.trait_disassemble();
        auto& buffer = _ct.db->buffers[commit.buffer_index];
        switch (table_id) {
          case tbl_id<StudentTable>(): {
            auto new_tuple = StudentTable::entry_type{};
            auto& table_ref = _ct.db->students;
            std::memcpy(&new_tuple, buffer.data(), sizeof(new_tuple));
            table_ref.write(new_tuple);
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
    _ct.db->buffers.release(commit.buffer_index);
    commit_it++;
  }
  job_ack(OpAck::SUCCESS, _ct, _parser, _job);
  _ct.db->buffers.release(_job.buffer_index_);
  return;
}

void WorkerThread::release_locks(const WorkerContext& _ct,
                                 const WorkerRuntime& _rt) noexcept {
  auto lock_it = _rt.lock_table.begin();
  auto lock_it_end = _rt.lock_table.end();
  while (lock_it != lock_it_end) {
    auto& lock = *lock_it;
    auto [tbl, tp_key] = lock.key.disassemble();
    switch (tbl) {
      case tbl_id<StudentTable>(): {
        auto& table_ref = _ct.db->students;
        table_ref.release(tp_key);
        break;
      }
      default: {
        break;
      }
    }
    lock_it++;
  }
}

void WorkerThread::worker_quit(const WorkerContext& _ct,
                               const WorkerRuntime& _rt) noexcept {
  release_locks(_ct, _rt);
  _ct.running->clear(std::memory_order_release);
  _ct.main_ch->send(_ct.txn_id);
}

/**
 * @brief Main worker thread function
 *
 * Process loop:
 * - Wait for jobs
 * - Process requests
 * - Handle commits
 * - Manage cleanup
 *
 * @param _ct Worker thread context
 */
void WorkerThread::worker_main(const WorkerContext&& _ct) noexcept {
  const auto context = WorkerContext(std::move(_ct));
  auto runtime = WorkerRuntime{0, {}, TxnStatus::GROWING, {}};

  while (context.running->test_and_set(std::memory_order_relaxed)) {
    auto job_r = _ct.job_ch->recv();
    if (!job_r) {
      runtime.timeout_cnt++;
      continue;
    } else {
      runtime.timeout_cnt = 0;
    }
    if (runtime.timeout_cnt >= MAX_TIMEOUT_RETRY) {
      worker_quit(context, runtime);
      return;
    }
    auto job = job_r.unwrap();
    auto parser = Parser(_ct.db->buffers[job.buffer_index_], job.buffer_size_);

    auto op_r = parser.get_op();
    if (!op_r) {
      job_ack(OpAck::BAD_OP, context, parser, job);
      _ct.db->buffers.release(job.buffer_index_);
      continue;
    }
    auto op = op_r.unwrap();
    switch (op) {
      case OpType::START_TXN:
        parser.set_txn_id(_ct.txn_id);
        job_ack(OpAck::SUCCESS, context, parser, job);
        _ct.db->buffers.release(job.buffer_index_);
        break;
      case OpType::COMMIT_TXN: {
        process_commit(context, runtime, parser, job);
        worker_quit(context, runtime);
        return;
      }
      case OpType::ADD_TUPLE: {
        process_add(context, runtime, parser, job);
        break;
      }
      case OpType::SHARED_READ: {
        process_shared_read(context, runtime, parser, job);
        break;
      }
      case OpType::EXCLUSIVE_READ: {
        process_exclusive_read(context, runtime, parser, job);
        break;
      }
      case OpType::YIELD_READ: {
        process_yield(context, runtime, parser, job);
        break;
      }
      case OpType::PROMOTE: {
        process_promote(context, runtime, parser, job);
        break;
      }
      case OpType::UPDATE: {
        process_update(context, runtime, parser, job);
        break;
      }
      case OpType::DELETE: {
        process_rm(context, runtime, parser, job);
        break;
      }
      default: {
        job_ack(OpAck::BAD_OP, context, parser, job);
        continue;
      }
    }
  }
  worker_quit(context, runtime);
  return;
}

}  // namespace PawnDB
