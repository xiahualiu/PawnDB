/**
 * @file worker_thread.h
 * @author Xiahua Liu @xiahualiu
 * @brief PawnDB worker class, used for processing transactions.
 * @version 0.1
 * @date 2025-01-02
 *
 * @copyright MIT License
 *
 */

#ifndef PAWNDB_THREAD_WORKER_H
#define PAWNDB_THREAD_WORKER_H

#include <sys/socket.h>

#include <atomic>
#include <cstdint>
#include <thread>

#include "pawndb/channel.h"
#include "pawndb/lock_table.h"
#include "pawndb/params.h"
#include "pawndb/parser.h"
#include "pawndb/schema/demo.h"

namespace PawnDB {

enum class TxnStatus : std::uint8_t { GROWING, SHRINKING, COMMITTED, ABORTED };

struct Job {
  sockaddr client_addr;
  socklen_t client_addr_len;
  buf_size_t buffer_size;
  tbl_row_t buffer_index;
};

struct Commit {
  OpType op;
  tp_id_t tbl_id;
  tbl_row_t tp_key;
  tbl_row_t buffer_index;
};

using MainChannel = Channel<txn_id_t>;
using JobChannel = Channel<Job>;

struct WorkerContext {
  std::atomic_flag* running;
  JobChannel* job_ch;
  MainChannel* main_ch;
  Database* db;
  txn_id_t txn_id;
  int server_fd;

  WorkerContext(std::atomic_flag* _running, JobChannel* _job_ch,
                MainChannel* _main_ch, Database* _db, txn_id_t _txn_id,
                int _server_fd) noexcept
      : running(_running),
        job_ch(_job_ch),
        main_ch(_main_ch),
        db(_db),
        txn_id(_txn_id),
        server_fd(_server_fd) {}
};

struct WorkerRuntime {
  std::uint8_t timeout_cnt;
  Queue<Commit, MAX_COMMIT_PER_TRANSACTION> commits;
  TxnStatus status;
  LockTable lock_table;
};

void worker_main(const WorkerContext&& _context) noexcept;

class WorkerThread {
 public:
  WorkerThread() = default;

  WorkerThread(const WorkerContext& _context) noexcept {
    thread = std::thread(worker_main, std::move(_context));
  }

  std::thread thread;
};

}  // namespace PawnDB

#endif
