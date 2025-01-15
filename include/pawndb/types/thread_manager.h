#ifndef PAWNDB_THREAD_MANAGER_H
#define PAWNDB_THREAD_MANAGER_H

#include <sys/socket.h>

#include <atomic>
#include <thread>

#include "pawndb/params.h"
#include "pawndb/schema/demo.h"
#include "pawndb/traits/parser.h"
#include "pawndb/traits/thread.h"
#include "pawndb/types/parser.h"
#include "pawndb/types/ret_channel.h"
#include "pawndb/types/worker_table.h"

namespace PawnDB {

class ThreadManager : public ThreadTrait<ThreadManager> {
 public:
  ThreadManager(Database* _db, int _server_fd) noexcept;

  // Non-copyable
  ThreadManager(const ThreadManager& _other) = delete;
  ThreadManager& operator=(const ThreadManager& _other) = delete;

  /** @brief Start main thread */
  void trait_start() noexcept;

  /** @brief Stop main thread */
  void trait_stop() noexcept;

  /** @brief Join main thread */
  void trait_join() noexcept;

  /** @brief Check if main thread is running */
  bool trait_is_running() const noexcept;

 private:
  /** @brief Send reply to client */
  void reply(OpAck _ack, Parser& _parser, const sockaddr& _client_addr,
             const socklen_t _client_addr_len) noexcept;

  // Database instance
  Database* db_;

  // Query worker
  WorkerTable worker_query;

  // Pre-allocate worker threads and all associated resources
  RetChannel ret_channel;
  std::array<std::thread, MAX_TRANSACTIONS> workers;
  std::array<std::atomic_flag, MAX_TRANSACTIONS> running_flags;
  std::array<JobChannel, MAX_TRANSACTIONS> job_channels;
  const int server_fd_;
};

}  // namespace PawnDB

#endif
