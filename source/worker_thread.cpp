/**
 * @file worker_thread.cpp
 * @author Xiahua Liu @xiahualiu
 * @brief PawnDB worker class, used for processing transactions.
 * @version 0.1
 * @date 2025-01-02
 *
 * @copyright MIT License
 *
 */

#include "pawndb/worker_thread.h"

#include <cstring>

#include "pawndb/params.h"
#include "pawndb/parser.h"
#include "pawndb/table_types.h"
#include "pawndb/tuple_serdes.h"

namespace PawnDB {

static void job_ack(const OpAck _ack, const WorkerContext& _context,
                    Parser& _parser, Job& _job) noexcept {
  _parser.set_ack(_ack);
  sendto(_context.server_fd, _parser.buffer().data(), _parser.get_buffer_size(),
         0, &_job.client_addr, _job.client_addr_len);
  _context.db->buffers.release(_job.buffer_index);
  _context.job_ch->pop();
}

static void process_add(const WorkerContext& _context, WorkerRuntime& _runtime,
                        Parser& _parser, Job& _job) {
  if (_runtime.status == TxnStatus::GROWING) {
    _runtime.status = TxnStatus::SHRINKING;
  }
  if (_runtime.commits.full()) {
    _parser.set_buffer_size(7);
    job_ack(OpAck::COMMIT_FULL, _context, _parser, _job);
    return;
  }
  auto table_id_r = _parser.get_tbl();
  if (!table_id_r) {
    job_ack(OpAck::BAD_TABLE, _context, _parser, _job);
    return;
  }
  auto table_id = table_id_r.unwrap();
  auto new_buffer_index = _context.db->buffers.request();
  switch (table_id) {
    case tbl_id<student_table>(): {
      auto new_tuple = std::tuple<student_name, std::uint8_t>{};
      auto serializer = TpSerDes<student_table>();
      if (!serializer.deserialize(_context.db->buffers[_job.buffer_index],
                                  _parser.get_data_offset(), new_tuple)) {
        _parser.set_buffer_size(7);
        job_ack(OpAck::BAD_DATA, _context, _parser, _job);
        _context.db->buffers.release(new_buffer_index);
        return;
      }
      std::memcpy(_context.db->buffers[new_buffer_index].data(), &new_tuple,
                  sizeof(new_tuple));
      break;
    }
    default: {
      _parser.set_buffer_size(7);
      job_ack(OpAck::BAD_TABLE, _context, _parser, _job);
      _context.db->buffers.release(new_buffer_index);
      return;
    }
  }
  _runtime.commits.push(
      Commit{OpType::ADD_TUPLE, table_id, 0, new_buffer_index});
  job_ack(OpAck::SUCCESS, _context, _parser, _job);
  return;
}

static void process_rm(const WorkerContext& _context, WorkerRuntime& _runtime,
                       Parser& _parser, Job& _job) {
  if (_runtime.status == TxnStatus::GROWING) {
    _runtime.status = TxnStatus::SHRINKING;
  }
  if (_runtime.commits.full()) {
    _parser.set_buffer_size(7);
    job_ack(OpAck::COMMIT_FULL, _context, _parser, _job);
    return;
  }
  auto table_id_r = _parser.get_tbl();
  if (!table_id_r) {
    _parser.set_buffer_size(7);
    job_ack(OpAck::BAD_TABLE, _context, _parser, _job);
    return;
  }
  auto tuple_key_r = _parser.get_tp_key();
  if (!tuple_key_r) {
    _parser.set_buffer_size(7);
    job_ack(OpAck::BAD_TP, _context, _parser, _job);
    return;
  }
  auto table_id = table_id_r.unwrap();
  auto tuple_key = tuple_key_r.unwrap();
  auto lock_r = _runtime.lock_table.get(table_id, tuple_key);
  if (!lock_r || lock_r.unwrap() != LockType::Exclusive) {
    _parser.set_buffer_size(7);
    job_ack(OpAck::BAD_ACCESS, _context, _parser, _job);
    return;
  }
  _runtime.commits.push(Commit{OpType::DELETE, table_id, tuple_key, 0});
  job_ack(OpAck::SUCCESS, _context, _parser, _job);
  return;
}

static void process_shared_read(const WorkerContext& _context,
                                WorkerRuntime& _runtime, Parser& _parser,
                                Job& _job) {
  if (_runtime.status != TxnStatus::GROWING) {
    _parser.set_buffer_size(7);
    job_ack(OpAck::BAD_PHASE, _context, _parser, _job);
    return;
  }
  auto table_id_r = _parser.get_tbl();
  if (!table_id_r) {
    _parser.set_buffer_size(7);
    job_ack(OpAck::BAD_TABLE, _context, _parser, _job);
    return;
  }
  auto tuple_key_r = _parser.get_tp_key();
  if (!tuple_key_r) {
    _parser.set_buffer_size(7);
    job_ack(OpAck::BAD_TP, _context, _parser, _job);
    return;
  }
  auto table_id = table_id_r.unwrap();
  auto tuple_key = tuple_key_r.unwrap();
  if (_runtime.lock_table.get(table_id, tuple_key)) {
    _parser.set_buffer_size(7);
    job_ack(OpAck::BAD_ACCESS, _context, _parser, _job);
    return;
  }
  switch (table_id) {
    case tbl_id<student_table>(): {
      auto& table_ref = std::get<0>(_context.db->table);
      tbl_row_t tuple_i = 0;
      auto timeout_cnt = 0;
      while (true) {
        if (timeout_cnt >= MAX_TIMEOUT_RETRY) {
          job_ack(OpAck::TIMEOUT, _context, _parser, _job);
          return;
        }
        if (!_context.running->test_and_set(std::memory_order_acquire)) {
          _context.running->clear(std::memory_order_release);
          job_ack(OpAck::ABORTED, _context, _parser, _job);
          return;
        }
        auto wait_r = table_ref.wait_s();
        if (!wait_r) {
          timeout_cnt++;
          continue;
        }
        tuple_i = wait_r.unwrap();
        break;
      }
      _runtime.lock_table.lock(tbl_id<student_table>(),
                               table_ref.key_at(tuple_i), LockType::Shared);
      _parser.set_tp_key(table_ref.key_at(tuple_i));
      auto serializer = TpSerDes<student_table>();
      auto offset = serializer.serialize(table_ref[tuple_i], _parser.buffer(),
                                         _parser.get_data_offset());
      _parser.set_data_size(offset - _parser.get_data_offset());
      _parser.set_buffer_size(offset);
      job_ack(OpAck::SUCCESS, _context, _parser, _job);
      return;
    }
    default: {
      _parser.set_buffer_size(7);
      job_ack(OpAck::BAD_TABLE, _context, _parser, _job);
      return;
    }
  }
}

static void process_exclusive_read(const WorkerContext& _context,
                                   WorkerRuntime& _runtime, Parser& _parser,
                                   Job& _job) {
  if (_runtime.status != TxnStatus::GROWING) {
    _parser.set_buffer_size(7);
    job_ack(OpAck::BAD_PHASE, _context, _parser, _job);
    return;
  }
  auto table_id_r = _parser.get_tbl();
  if (!table_id_r) {
    _parser.set_buffer_size(7);
    job_ack(OpAck::BAD_TABLE, _context, _parser, _job);
    return;
  }
  auto tuple_key_r = _parser.get_tp_key();
  if (!tuple_key_r) {
    _parser.set_buffer_size(7);
    job_ack(OpAck::BAD_TP, _context, _parser, _job);
    return;
  }
  auto table_id = table_id_r.unwrap();
  auto tuple_key = tuple_key_r.unwrap();
  if (_runtime.lock_table.get(table_id, tuple_key)) {
    _parser.set_buffer_size(7);
    job_ack(OpAck::BAD_ACCESS, _context, _parser, _job);
    return;
  }
  switch (table_id) {
    case tbl_id<student_table>(): {
      auto& table_ref = std::get<0>(_context.db->table);
      tbl_row_t tuple_i = 0;
      std::uint8_t timeout_cnt = 0;
      while (true) {
        if (timeout_cnt >= MAX_TIMEOUT_RETRY) {
          job_ack(OpAck::TIMEOUT, _context, _parser, _job);
          return;
        }
        if (!_context.running->test_and_set(std::memory_order_acquire)) {
          _context.running->clear(std::memory_order_release);
          job_ack(OpAck::ABORTED, _context, _parser, _job);
          return;
        }
        auto wait_r = table_ref.wait_x();
        if (!wait_r) {
          timeout_cnt++;
          continue;
        }
        tuple_i = wait_r.unwrap();
        break;
      }
      _runtime.lock_table.lock(tbl_id<student_table>(),
                               table_ref.key_at(tuple_i), LockType::Exclusive);
      _parser.set_tp_key(table_ref.key_at(tuple_i));
      auto serializer = TpSerDes<student_table>();
      auto offset = serializer.serialize(table_ref[tuple_i], _parser.buffer(),
                                         _parser.get_data_offset());
      _parser.set_data_size(offset - _parser.get_data_offset());
      _parser.set_buffer_size(offset);
      job_ack(OpAck::SUCCESS, _context, _parser, _job);
      return;
    }
    default: {
      _parser.set_buffer_size(7);
      job_ack(OpAck::BAD_TABLE, _context, _parser, _job);
      return;
    }
  }
}

static void process_update(const WorkerContext& _context,
                           WorkerRuntime& _runtime, Parser& _parser,
                           Job& _job) {
  if (_runtime.status == TxnStatus::GROWING) {
    _runtime.status = TxnStatus::SHRINKING;
  }
  if (_runtime.commits.full()) {
    _parser.set_buffer_size(7);
    job_ack(OpAck::COMMIT_FULL, _context, _parser, _job);
    return;
  }
  auto table_id_r = _parser.get_tbl();
  if (!table_id_r) {
    _parser.set_buffer_size(7);
    job_ack(OpAck::BAD_TABLE, _context, _parser, _job);
    return;
  }
  auto tuple_key_r = _parser.get_tp_key();
  if (!tuple_key_r) {
    _parser.set_buffer_size(7);
    job_ack(OpAck::BAD_TP, _context, _parser, _job);
    return;
  }
  auto table_id = table_id_r.unwrap();
  auto tuple_key = tuple_key_r.unwrap();
  auto lock_r = _runtime.lock_table.get(table_id, tuple_key);
  if (!lock_r || lock_r.unwrap() != LockType::Exclusive) {
    _parser.set_buffer_size(7);
    job_ack(OpAck::BAD_ACCESS, _context, _parser, _job);
    return;
  }
  auto new_buffer_index = _context.db->buffers.request();
  switch (table_id) {
    case tbl_id<student_table>(): {
      auto serializer = TpSerDes<student_table>();
      auto new_tuple = std::tuple<student_name, std::uint8_t>{};
      if (!serializer.deserialize(_context.db->buffers[_job.buffer_index],
                                  _parser.get_data_offset(), new_tuple)) {
        _parser.set_buffer_size(7);
        job_ack(OpAck::BAD_DATA, _context, _parser, _job);
        _context.db->buffers.release(new_buffer_index);
        return;
      }
      std::memcpy(_context.db->buffers[new_buffer_index].data(), &new_tuple,
                  sizeof(new_tuple));
      break;
    }
    default: {
      _parser.set_buffer_size(7);
      job_ack(OpAck::BAD_TABLE, _context, _parser, _job);
      _context.db->buffers.release(new_buffer_index);
      return;
    }
  }
  _runtime.commits.push(
      Commit{OpType::UPDATE, table_id, tuple_key, new_buffer_index});
  job_ack(OpAck::SUCCESS, _context, _parser, _job);
  return;
}

static void process_promote(const WorkerContext& _context,
                            WorkerRuntime& _runtime, Parser& _parser,
                            Job& _job) {
  if (_runtime.status != TxnStatus::GROWING) {
    _parser.set_buffer_size(7);
    job_ack(OpAck::BAD_PHASE, _context, _parser, _job);
    return;
  }
  auto table_id_r = _parser.get_tbl();
  if (!table_id_r) {
    _parser.set_buffer_size(7);
    job_ack(OpAck::BAD_TABLE, _context, _parser, _job);
    return;
  }
  auto tuple_key_r = _parser.get_tp_key();
  if (!tuple_key_r) {
    _parser.set_buffer_size(7);
    job_ack(OpAck::BAD_TP, _context, _parser, _job);
    return;
  }
  auto table_id = table_id_r.unwrap();
  auto tuple_key = tuple_key_r.unwrap();
  if (_runtime.lock_table.promote(table_id, tuple_key) != LockError::None) {
    _parser.set_buffer_size(7);
    job_ack(OpAck::BAD_ACCESS, _context, _parser, _job);
    return;
  }
  switch (table_id) {
    case tbl_id<student_table>(): {
      auto& table_ref = std::get<0>(_context.db->table);
      auto timeout_cnt = 0;
      while (true) {
        if (timeout_cnt >= MAX_TIMEOUT_RETRY) {
          job_ack(OpAck::TIMEOUT, _context, _parser, _job);
          return;
        }
        if (!_context.running->test_and_set(std::memory_order_acquire)) {
          _context.running->clear(std::memory_order_release);
          job_ack(OpAck::ABORTED, _context, _parser, _job);
          return;
        }
        auto wait_r = table_ref.promote(tuple_key);
        if (wait_r != TableError::None) {
          timeout_cnt++;
          continue;
        }
        break;
      }
      job_ack(OpAck::SUCCESS, _context, _parser, _job);
      return;
    }
    default: {
      _parser.set_buffer_size(7);
      job_ack(OpAck::BAD_TABLE, _context, _parser, _job);
      return;
    }
  }
}

static void process_commit(const WorkerContext& _context,
                           WorkerRuntime& _runtime, Parser& _parser,
                           Job& _job) {
  if (_runtime.status != TxnStatus::SHRINKING) {
    job_ack(OpAck::SUCCESS, _context, _parser, _job);
    return;
  }
  while (!_runtime.commits.empty()) {
    auto commit = _runtime.commits.front();
    switch (commit.op) {
      case OpType::ADD_TUPLE: {
        auto table_id = commit.tbl_id;
        auto buffer_index = commit.buffer_index;
        switch (table_id) {
          case tbl_id<student_table>(): {
            auto new_tuple = student_table::tuple_type{};
            auto& table_ref = std::get<0>(_context.db->table);
            std::memcpy(&new_tuple, _context.db->buffers[buffer_index].data(),
                        sizeof(new_tuple));
            table_ref.insert(new_tuple);
            table_ref.notify_not_empty();
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
            auto& table_ref = std::get<0>(_context.db->table);
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
        auto table_id = commit.tbl_id;
        auto tuple_key = commit.tp_key;
        auto buffer_index = commit.buffer_index;
        switch (table_id) {
          case tbl_id<student_table>(): {
            auto new_tuple = student_table::tuple_type{};
            auto& table_ref = std::get<0>(_context.db->table);
            std::memcpy(&new_tuple, _context.db->buffers[buffer_index].data(),
                        sizeof(new_tuple));
            table_ref.update(tuple_key, new_tuple);
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
    _runtime.commits.pop();
  }
  job_ack(OpAck::SUCCESS, _context, _parser, _job);
  return;
}

static void release_locks(const WorkerContext& _context,
                          const WorkerRuntime& _runtime) {
  for (const auto [tbl, key, lock] : _runtime.lock_table) {
    switch (tbl) {
      case tbl_id<student_table>(): {
        auto& table = std::get<0>(_context.db->table);
        if (lock == LockType::Shared) {
          table.release_s(key);
        } else {
          table.release_x(key);
        }
        break;
      }
      default: {
        break;
      }
    }
  }
}

static void worker_quit(const WorkerContext& _context,
                        const WorkerRuntime& _runtime) noexcept {
  release_locks(_context, _runtime);
  _context.running->clear(std::memory_order_release);
  _context.main_ch->send(_context.txn_id);
}

void worker_main(const WorkerContext&& _context) noexcept {
  const auto context = WorkerContext(std::move(_context));
  auto runtime = WorkerRuntime{0, Queue<Commit, MAX_COMMIT_PER_TRANSACTION>{},
                               TxnStatus::GROWING, LockTable{}};

  while (context.running->test_and_set(std::memory_order_acquire)) {
    auto job_r = context.job_ch->recv();
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
    auto& job_buffer = context.db->buffers[job.buffer_index];
    auto parser = Parser{job_buffer, job.buffer_size};

    auto op_r = parser.get_op();
    if (!op_r) {
      job_ack(OpAck::BAD_OP, context, parser, job);
      continue;
    }
    auto op = op_r.unwrap();
    switch (op) {
      case OpType::START_TXN:
        parser.set_txn_id(context.txn_id);
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
