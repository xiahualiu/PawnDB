#ifndef PAWNDB_THREAD_MANAGER_H
#define PAWNDB_THREAD_MANAGER_H

#include <atomic>

#include "pawndb/schema/demo.h"
#include "pawndb/types/client_conn.h"
#include "pawndb/types/job_buf.h"
#include "pawndb/types/ret_channel.h"
#include "pawndb/types/worker_table.h"

namespace PawnDB {

class ThreadManager {
 public:
  /** @brief Constructor
   *  @param _db Database instance */
  ThreadManager(Database* _db) noexcept;

  // Non-copyable
  ThreadManager(const ThreadManager& _other) = delete;
  ThreadManager& operator=(const ThreadManager& _other) = delete;

  /** @brief Start main thread */
  void start_() noexcept;

  /** @brief Stop main thread */
  void stop_() noexcept;

  /** @brief Join main thread */
  // void join_() noexcept;

  /** @brief Check if main thread is running */
  bool is_running_() noexcept;

 private:
  /** @brief Send reply to client */
  void reply(OpAck _ack, job_buf& _parser, const client_conn& _conn) noexcept;

  /** @brief Database instance */
  Database& db_;

  /** @brief Return channel */
  ret_channel ret_ch_;

  /** @brief Worker hash table */
  WorkerTable workers_;

  /** @brief UDP server socket */
  int server_fd_;

  /** @brief Main thread running flag */
  std::atomic_flag running_;
};

}  // namespace PawnDB

#endif