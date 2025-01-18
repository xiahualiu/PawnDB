#ifndef PAWNDB_THREAD_MANAGER_H
#define PAWNDB_THREAD_MANAGER_H

#include <sys/socket.h>

#include <atomic>

#include "pawndb/schema/demo.h"
#include "pawndb/traits/parser.h"
#include "pawndb/traits/thread.h"
#include "pawndb/types/parser.h"
#include "pawndb/types/ret_channel.h"
#include "pawndb/types/worker_table.h"

namespace PawnDB {

class ThreadManager : public ThreadTrait<ThreadManager> {
 public:
  /** @brief Constructor
   *  @param _db Database instance */
  ThreadManager(Database* _db) noexcept;

  // Non-copyable
  ThreadManager(const ThreadManager& _other) = delete;
  ThreadManager& operator=(const ThreadManager& _other) = delete;

  /** @brief Start main thread */
  void trait_start() noexcept;

  /** @brief Stop main thread */
  void trait_stop() noexcept;

  /** @brief Join main thread */
  // void trait_join() noexcept;

  /** @brief Check if main thread is running */
  bool trait_is_running() noexcept;

 private:
  /** @brief Send reply to client */
  void reply(OpAck _ack, Parser& _parser, const sockaddr_un& _client_addr,
             const socklen_t _client_addr_len) noexcept;

  /** @brief Database instance */
  Database& db_;

  /** @brief Return channel */
  RetChannel ret_ch_;

  /** @brief Worker hash table */
  WorkerTable workers_;

  /** @brief UDP server socket */
  int server_fd_;

  /** @brief Main thread running flag */
  std::atomic_flag running_;
};

}  // namespace PawnDB

#endif
