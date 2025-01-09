/**
 * @file main_thread.cpp
 * @author Xiahua Liu @xiahualiu
 * @brief PawnDB main thread, used for managing worker threads and connections.
 * @version 0.1
 * @date 2025-01-02
 *
 * @copyright MIT License
 *
 */

#ifndef PAWNDB_MAIN_THREAD_H
#define PAWNDB_MAIN_THREAD_H

#include <unistd.h>

#include <thread>

#include "pawndb/schema/demo.h"
#include "pawndb/traits/thread.h"

namespace PawnDB {

struct MainContext {
  Database* db;
  int server_fd;
};

class MainThread : public ThreadTrait<MainThread> {
 public:
  MainThread(Database* _db, int _server_fd) noexcept
      : context_{_db, _server_fd} {}

  void trait_start() noexcept;
  void trait_stop() noexcept;
  bool trait_is_running() const noexcept;

  MainContext context_;
  std::thread main_thread_;
};

}  // namespace PawnDB

#endif
