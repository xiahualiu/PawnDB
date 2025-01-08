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

#include <iostream>
#include <thread>

#include "pawndb/params.h"
#include "pawndb/schema/demo.h"
#include "pawndb/traits/thread.h"
#include "pawndb/types/parser.h"
#include "pawndb/types/ret_channel.h"
#include "pawndb/types/worker_table.h"

namespace PawnDB {

struct MainContext {
  Database* db;
  int server_fd;
};

class MainThread : public ThreadTrait<MainThread> {
 public:
  MainThread(Database* _db, int _server_fd) noexcept
      : context_{_db, _server_fd} {}

  void trait_start() noexcept {
    main_thread_ = std::thread(&MainThread::run, context_);
  }

  void trait_stop() noexcept {
    std::cout << "Shutting down server..." << std::endl;
    shutdown(context_.server_fd, SHUT_RDWR);
    close(context_.server_fd);
    unlink(UNIX_SOCKET_PATH);
    main_thread_.join();
  }

  bool trait_is_running() const noexcept { return main_thread_.joinable(); }

  static void packet_ack(OpAck _ack, Parser& _parser, const int _server_fd,
                         sockaddr& _client_addr,
                         socklen_t _client_addr_len) noexcept;

  static void run(MainContext* _ct) noexcept;

  static void clean_worker(RetChannel& _ret, WorkerTable& _query,
                           MainContext& _ct) noexcept;

 private:
  MainContext context_;
  std::thread main_thread_;
};

}  // namespace PawnDB

#endif
