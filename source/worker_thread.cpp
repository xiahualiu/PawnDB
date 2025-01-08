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

#include <cstring>

#include "pawndb/params.h"
#include "pawndb/table_types.h"
#include "pawndb/traits/parser.h"
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
    return;
  }
  auto table_id_r = _parser.get_tbl();
  if (!table_id_r) {
    _parser.set_buffer_size(7);
    job_ack(OpAck::BAD_TABLE, _ct, _parser, _job);
    return;
  }
  auto table_id = table_id_r.unwrap();
  auto new_buffer_index_r = _ct.db->buffers.request();
  if (!new_buffer_index_r) {
    _parser.set_buffer_size(7);
    job_ack(OpAck::BUSY, _ct, _parser, _job);
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
        return;
      }
      auto new_entry = StudentTableEntry{new_tuple, 0, 0, false, false};
      std::memcpy(new_buffer_ref.data(), &new_entry, sizeof(new_entry));
      break;
    }
    default: {
      _parser.set_buffer_size(7);
      job_ack(OpAck::BAD_TABLE, _ct, _parser, _job);
      return;
    }
  }
  _rt.commit_table.insert({{table_id, tuple_key},
                           OpType::ADD_TUPLE,
                           new_buffer_index,
                           sizeof(StudentTuple),
                           false,
                           false});
  job_ack(OpAck::SUCCESS, _ct, _parser, _job);
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
    return;
  }
  auto table_id_r = _parser.get_tbl();
  if (!table_id_r) {
    _parser.set_buffer_size(7);
    job_ack(OpAck::BAD_TABLE, _ct, _parser, _job);
    return;
  }
  auto tuple_key_r = _parser.get_key();
  if (!tuple_key_r) {
    _parser.set_buffer_size(7);
    job_ack(OpAck::BAD_TP, _ct, _parser, _job);
    return;
  }
  auto table_id = table_id_r.unwrap();
  auto tuple_key = tuple_key_r.unwrap();
  auto lock_record_key = TableTupleKey{table_id, tuple_key};
  auto lock_r = _rt.lock_table.get_lock(lock_record_key);
  if (!lock_r || lock_r.unwrap() != LockType::Exclusive) {
    _parser.set_buffer_size(7);
    job_ack(OpAck::BAD_ACCESS, _ct, _parser, _job);
    return;
  }
  _rt.commit_table.insert(
      {lock_record_key, OpType::DELETE, 0, 0, false, false});
  job_ack(OpAck::SUCCESS, _ct, _parser, _job);
  return;
}

void WorkerThread::process_shared_read(const WorkerContext& _ct,
                                       WorkerRuntime& _rt, Parser& _parser,
                                       Job& _job) noexcept {
  if (_rt.status != TxnStatus::GROWING) {
    _parser.set_buffer_size(7);
    job_ack(OpAck::BAD_PHASE, _ct, _parser, _job);
    return;
  }
  auto table_id_r = _parser.get_tbl();
  if (!table_id_r) {
    _parser.set_buffer_size(7);
    job_ack(OpAck::BAD_TABLE, _ct, _parser, _job);
    return;
  }
  auto tuple_key_r = _parser.get_key();
  if (!tuple_key_r) {
    _parser.set_buffer_size(_parser, 7);
    job_ack(OpAck::BAD_TP, _ct, _parser, _job);
    return;
  }
  auto table_id = table_id_r.unwrap();
  auto tuple_key = tuple_key_r.unwrap();
  if (_rt.lock_table.get_lock({table_id, tuple_key})) {
    _parser.set_buffer_size(7);
    job_ack(OpAck::BAD_ACCESS, _ct, _parser, _job);
    return;
  }
  switch (table_id) {
    case tbl_id<StudentTable>(): {
      auto& table_ref = _ct.db->students;
      auto timeout_cnt = 0;
      while (true) {
        if (timeout_cnt >= MAX_TIMEOUT_RETRY) {
          job_ack(OpAck::TIMEOUT, _ct, _parser, _job);
          return;
        }
        if (!_ct.running->test_and_set(std::memory_order_acquire)) {
          _ct.running->clear(std::memory_order_release);
          job_ack(OpAck::ABORTED, _ct, _parser, _job);
          return;
        }
        auto wait_r = table_ref.wait_shared();
        if (!wait_r) {
          timeout_cnt++;
          continue;
        }
        auto student_entry = wait_r.unwrap();
        _rt.lock_table.lock({table_id, student_entry.key}, LockType::Shared);
        _parser.set_key(student_entry.key);
        auto offset = student_entry.tuple.serialize(_parser.get_buffer(), _parser.get_tuple_offset());
        _parser.set_buffer_size(_parser.get_tuple_offset() + offset);
        job_ack(OpAck::SUCCESS, _ct, _parser, _job);
        break;
      }
    }
    default: {
      set_buffer_size(_parser, 7);
      job_ack(OpAck::BAD_TABLE, _ct, _parser, _job);
      return;
    }
  }
}

void WorkerThread::process_exclusive_read(const WorkerContext& _ct,
                                          WorkerRuntime& _rt,
                                          ParserStruct& _parser,
                                          Job& _job) noexcept {
  if (_rt.status != TxnStatus::GROWING) {
    set_buffer_size(_parser, 7);
    job_ack(OpAck::BAD_PHASE, _ct, _parser, _job);
    return;
  }
  auto table_id_r = get_tbl(_parser);
  if (!table_id_r) {
    set_buffer_size(_parser, 7);
    job_ack(OpAck::BAD_TABLE, _ct, _parser, _job);
    return;
  }
  auto tuple_key_r = get_tp_key(_parser);
  if (!tuple_key_r) {
    set_buffer_size(_parser, 7);
    job_ack(OpAck::BAD_TP, _ct, _parser, _job);
    return;
  }
  auto table_id = table_id_r.unwrap();
  auto tuple_key = tuple_key_r.unwrap();
  if (_lk_func::get(_rt.lock_table, table_id, tuple_key)) {
    set_buffer_size(_parser, 7);
    job_ack(OpAck::BAD_ACCESS, _ct, _parser, _job);
    return;
  }
  switch (table_id) {
    case tbl_id<student_table>(): {
      auto& table_ref = std::get<0>(_ct.db->table);
      tbl_row_t tuple_i = 0;
      std::uint8_t timeout_cnt = 0;
      while (true) {
        if (timeout_cnt >= MAX_TIMEOUT_RETRY) {
          job_ack(OpAck::TIMEOUT, _ct, _parser, _job);
          return;
        }
        if (!_ct.running->test_and_set(std::memory_order_acquire)) {
          _ct.running->clear(std::memory_order_release);
          job_ack(OpAck::ABORTED, _ct, _parser, _job);
          return;
        }
        auto wait_r = student_table_func::wait_x(table_ref);
        if (!wait_r) {
          timeout_cnt++;
          continue;
        }
        tuple_i = wait_r.unwrap();
        break;
      }
      lock(_rt.lock_table, tbl_id<student_table>(),
           table_ref.hash.table[tuple_i].key, LockType::Exclusive);
      set_tp_key(_parser, table_ref.hash.table[tuple_i].key);
      auto serializer = TpSerDes<student_table>();
      auto offset = serializer.serialize(table_ref.tuples[tuple_i],
                                         _parser.data, get_data_offset());
      set_data_size(_parser, offset - get_data_offset());
      set_buffer_size(_parser, offset);
      job_ack(OpAck::SUCCESS, _ct, _parser, _job);
      return;
    }
    default: {
      set_buffer_size(_parser, 7);
      job_ack(OpAck::BAD_TABLE, _ct, _parser, _job);
      return;
    }
  }
}

void WorkerThread::process_yield(const WorkerContext& _ct, WorkerRuntime& _rt,
                                 ParserStruct& _parser, Job& _job) noexcept {
  if (_rt.status != TxnStatus::GROWING) {
    set_buffer_size(_parser, 7);
    job_ack(OpAck::BAD_PHASE, _ct, _parser, _job);
    return;
  }
  auto table_id_r = get_tbl(_parser);
  if (!table_id_r) {
    set_buffer_size(_parser, 7);
    job_ack(OpAck::BAD_TABLE, _ct, _parser, _job);
    return;
  }
  auto tuple_key_r = get_tp_key(_parser);
  if (!tuple_key_r) {
    set_buffer_size(_parser, 7);
    job_ack(OpAck::BAD_TP, _ct, _parser, _job);
    return;
  }
  auto table_id = table_id_r.unwrap();
  auto tuple_key = tuple_key_r.unwrap();
  auto lock_r = _lk_func::get(_rt.lock_table, table_id, tuple_key);
  if (!lock_r || lock_r.unwrap() != LockType::Shared) {
    set_buffer_size(_parser, 7);
    job_ack(OpAck::BAD_ACCESS, _ct, _parser, _job);
    return;
  }
  unlock(_rt.lock_table, table_id, tuple_key);
  switch (table_id) {
    case tbl_id<student_table>(): {
      auto& table_ref = std::get<0>(_ct.db->table);
      student_table_func::release_s(table_ref, tuple_key);
      job_ack(OpAck::SUCCESS, _ct, _parser, _job);
      return;
    }
    default: {
      set_buffer_size(_parser, 7);
      job_ack(OpAck::BAD_TABLE, _ct, _parser, _job);
      return;
    }
  }
}

void WorkerThread::process_update(const WorkerContext& _ct, WorkerRuntime& _rt,
                                  ParserStruct& _parser, Job& _job) noexcept {
  if (_rt.status == TxnStatus::GROWING) {
    _rt.status = TxnStatus::SHRINKING;
  }
  if (_cmt_func::full(_rt.commits)) {
    set_buffer_size(_parser, 7);
    job_ack(OpAck::COMMIT_FULL, _ct, _parser, _job);
    return;
  }
  auto table_id_r = get_tbl(_parser);
  if (!table_id_r) {
    set_buffer_size(_parser, 7);
    job_ack(OpAck::BAD_TABLE, _ct, _parser, _job);
    return;
  }
  auto tuple_key_r = get_tp_key(_parser);
  if (!tuple_key_r) {
    set_buffer_size(_parser, 7);
    job_ack(OpAck::BAD_TP, _ct, _parser, _job);
    return;
  }
  auto table_id = table_id_r.unwrap();
  auto tuple_key = tuple_key_r.unwrap();
  auto lock_r = _lk_func::get(_rt.lock_table, table_id, tuple_key);
  if (!lock_r || lock_r.unwrap() != LockType::Exclusive) {
    set_buffer_size(_parser, 7);
    job_ack(OpAck::BAD_ACCESS, _ct, _parser, _job);
    return;
  }
  auto new_buffer_ref = request(_ct.db->buffers);
  switch (table_id) {
    case tbl_id<student_table>(): {
      auto serializer = TpSerDes<student_table>();
      auto new_tuple = std::tuple<student_name, std::uint8_t>{};
      if (!serializer.deserialize(*new_buffer_ref, get_data_offset(),
                                  new_tuple)) {
        set_buffer_size(_parser, 7);
        job_ack(OpAck::BAD_DATA, _ct, _parser, _job);
        return;
      }
      std::memcpy((*new_buffer_ref).data(), &new_tuple, sizeof(new_tuple));
      break;
    }
    default: {
      set_buffer_size(_parser, 7);
      job_ack(OpAck::BAD_TABLE, _ct, _parser, _job);
      return;
    }
  }
  _cmt_func::push(_rt.commits,
                  {OpType::UPDATE, table_id, tuple_key, new_buffer_ref});
  job_ack(OpAck::SUCCESS, _ct, _parser, _job);
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
                                   ParserStruct& _parser, Job& _job) noexcept {
  if (_rt.status != TxnStatus::GROWING) {
    set_buffer_size(_parser, 7);
    job_ack(OpAck::BAD_PHASE, _ct, _parser, _job);
    return;
  }
  auto table_id_r = get_tbl(_parser);
  if (!table_id_r) {
    set_buffer_size(_parser, 7);
    job_ack(OpAck::BAD_TABLE, _ct, _parser, _job);
    return;
  }
  auto tuple_key_r = get_tp_key(_parser);
  if (!tuple_key_r) {
    set_buffer_size(_parser, 7);
    job_ack(OpAck::BAD_TP, _ct, _parser, _job);
    return;
  }
  auto table_id = table_id_r.unwrap();
  auto tuple_key = tuple_key_r.unwrap();
  auto lock_r = _lk_func::get(_rt.lock_table, table_id, tuple_key);
  if (!lock_r || lock_r.unwrap() != LockType::Shared) {
    set_buffer_size(_parser, 7);
    job_ack(OpAck::BAD_ACCESS, _ct, _parser, _job);
    return;
  }
  switch (table_id) {
    case tbl_id<student_table>(): {
      auto& table_ref = std::get<0>(_ct.db->table);
      auto timeout_cnt = 0;
      while (true) {
        if (timeout_cnt >= MAX_TIMEOUT_RETRY) {
          job_ack(OpAck::TIMEOUT, _ct, _parser, _job);
          return;
        }
        if (!_ct.running->test_and_set(std::memory_order_acquire)) {
          _ct.running->clear(std::memory_order_release);
          job_ack(OpAck::ABORTED, _ct, _parser, _job);
          return;
        }
        auto wait_r = student_table_func::promote(table_ref, tuple_key);
        if (wait_r != TableError::None) {
          timeout_cnt++;
          continue;
        }
        break;
      }
      if (_lk_func::promote(_rt.lock_table, table_id, tuple_key) !=
          LockError::None) {
        set_buffer_size(_parser, 7);
        job_ack(OpAck::BAD_ACCESS, _ct, _parser, _job);
        return;
      }
      job_ack(OpAck::SUCCESS, _ct, _parser, _job);
      return;
    }
    default: {
      set_buffer_size(_parser, 7);
      job_ack(OpAck::BAD_TABLE, _ct, _parser, _job);
      return;
    }
  }
}

/**
 * @brief Processes transaction commit
 *
 * Handles commit phase by:
 * - Validating transaction is in shrinking phase
 * - Processing queued commit operations
 * - Releasing locks and buffers
 * - Sending acknowledgment
 *
 * Operation types:
 * - ADD_TUPLE: Insert new tuple
 * - DELETE_TUPLE: Remove existing tuple
 * - UPDATE: Modify existing tuple
 *
 * @param _ct Worker execution context
 * @param _rt Worker runtime state
 * @param _parser Request parser
 * @param _job Current job details
 */
void WorkerThread::process_commit(const WorkerContext& _ct, WorkerRuntime& _rt,
                                  ParserStruct& _parser, Job& _job) noexcept {
  if (_rt.status != TxnStatus::SHRINKING) {
    job_ack(OpAck::SUCCESS, _ct, _parser, _job);
    return;
  }
  while (!_cmt_func::empty(_rt.commits)) {
    auto commit = _cmt_func::front(_rt.commits);
    switch (commit.op) {
      case OpType::ADD_TUPLE: {
        auto table_id = commit.tbl_id;
        auto& buffer = commit.buffer;
        switch (table_id) {
          case tbl_id<student_table>(): {
            auto new_tuple = student_table_func::tuple_type{};
            auto& table_ref = std::get<0>(_ct.db->table);
            std::memcpy(&new_tuple, (*buffer).data(), sizeof(new_tuple));
            student_table_func::insert(table_ref, new_tuple);
            table_ref.not_empty.notify_all();
            table_ref.s_available.notify_all();
            table_ref.x_available.notify_one();
            break;
          }
          default: {
            break;
          }
        }
        break;
      }
      case OpType::DELETE: {
        auto table_id = commit.tbl_id;
        auto tuple_key = commit.tp_key;
        switch (table_id) {
          case tbl_id<student_table>(): {
            auto& table_ref = std::get<0>(_ct.db->table);
            student_table_func::remove(table_ref, tuple_key);
            table_ref.not_full.notify_one();
            break;
          }
          default: {
            break;
          }
        }
        break;
      }
      case OpType::UPDATE: {
        auto table_id = commit.tbl_id;
        auto tuple_key = commit.tp_key;
        auto& buffer = commit.buffer;
        switch (table_id) {
          case tbl_id<student_table>(): {
            auto new_tuple = student_table_func::tuple_type{};
            auto& table_ref = std::get<0>(_ct.db->table);
            std::memcpy(&new_tuple, (*buffer).data(), sizeof(new_tuple));
            student_table_func::update(table_ref, tuple_key, new_tuple);
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
    _cmt_func::pop(_rt.commits);
  }
  job_ack(OpAck::SUCCESS, _ct, _parser, _job);
  return;
}

/**
 * @brief Releases all locks held by transaction
 *
 * Iterates through lock table and releases:
 * - Shared locks
 * - Exclusive locks
 *
 * @param _ct Worker context containing transaction info
 * @param _rt Runtime state with lock table
 */
void WorkerThread::release_locks(const WorkerContext& _ct,
                                 const WorkerRuntime& _rt) noexcept {
  using _hash_func = HashFunc<key_t, MAX_LOCK_PER_TRANSACTION>;
  for (tbl_row_t i = 0; i < MAX_LOCK_PER_TRANSACTION; i++) {
    if (!_hash_func::is_valid(_rt.lock_table.hash, i)) {
      continue;
    }
    auto key = _rt.lock_table.hash.table[i].key;
    auto lock_type = _rt.lock_table.locks[i];
    auto [tbl, tp_key] = _lk_func::key(key);
    switch (tbl) {
      case tbl_id<student_table>(): {
        auto& table = std::get<0>(_ct.db->table);
        if (lock_type == LockType::Shared) {
          student_table_func::release_s(table, tp_key);
        } else {
          student_table_func::release_x(table, tp_key);
        }
        break;
      }
      default: {
        break;
      }
    }
  }
}

/**
 * @brief Performs worker thread cleanup and shutdown
 *
 * Cleanup sequence:
 * - Release all held locks
 * - Clear running flag
 * - Notify main thread
 *
 * @param _ct Worker context
 * @param _rt Runtime state
 */
void WorkerThread::worker_quit(const WorkerContext& _ct,
                               const WorkerRuntime& _rt) noexcept {
  release_locks(_ct, _rt);
  _ct.running->clear(std::memory_order_release);
  _txn_func::send(*_ct.main_ch, std::move(_ct.txn_id));
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
  auto runtime =
      WorkerRuntime{0, QueueData<Commit, MAX_COMMIT_PER_TRANSACTION>{},
                    TxnStatus::GROWING, LockRecords{}};

  while (context.running->test_and_set(std::memory_order_acquire)) {
    auto job_r = _job_func::recv(*_ct.job_ch);
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
    auto& job_buffer = job.buffer;
    auto parser = ParserStruct{*job_buffer, job.buffer_size};

    auto op_r = get_op(parser);
    if (!op_r) {
      job_ack(OpAck::BAD_OP, context, parser, job);
      continue;
    }
    auto op = op_r.unwrap();
    switch (op) {
      case OpType::START_TXN:
        set_txn_id(parser, context.txn_id);
        job_ack(OpAck::SUCCESS, context, parser, job);
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
