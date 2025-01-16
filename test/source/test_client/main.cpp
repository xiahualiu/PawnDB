/**
 * @file main.cpp
 * @brief Channal unit tests
 * @version 0.1
 * @date 2025-01-04
 *
 * @copyright Copyright (c) 2025
 *
 */

#define DOCTEST_CONFIG_IMPLEMENT

#include <sys/socket.h>
#include <sys/un.h>

#include <thread>

#include "doctest/doctest.h"
#include "pawndb/params.h"
#include "pawndb/types/parser.h"
#include "pawndb/types/thread_manager.h"
#include "pawndb/types/worker_table.h"

namespace PawnDB {

// Bad operation message
constexpr static auto bad_op =
    std::array<char, BUFFER_WIDTH>{static_cast<char>(OpType::MAX_OP_VALUE), 0};
constexpr static auto bad_op_size = 3;

// Start TXN
constexpr static auto start_txn = std::array<char, BUFFER_WIDTH>{
    static_cast<char>(OpType::START_TXN), 0, 0, 0, 0, 0, 0};
constexpr static auto start_txn_size = 7;

// Read as shared
constexpr static auto read_shared = std::array<char, BUFFER_WIDTH>{
    static_cast<char>(OpType::SHARED_READ), 0, 0, 0, 0, 0, 0};
constexpr static auto read_shared_size = 7;

// Read as exlusive
constexpr static auto read_exclusive = std::array<char, BUFFER_WIDTH>{
    static_cast<char>(OpType::EXCLUSIVE_READ), 0, 0, 0, 0, 0, 0};
constexpr static auto read_exlusive_size = 7;

// Abort transaction
constexpr static auto abort_txn = std::array<char, BUFFER_WIDTH>{
    static_cast<char>(OpType::COMMIT_TXN), 0, 0, 0, 0, 0, 0};
constexpr static auto abort_txn_size = 7;

// Commit transaction
constexpr static auto commit_txn = std::array<char, BUFFER_WIDTH>{
    static_cast<char>(OpType::COMMIT_TXN), 0, 0, 0, 0, 0, 0};
constexpr static auto commit_txn_size = 7;

// Remove tuple
constexpr static auto remove_tuple = std::array<char, BUFFER_WIDTH>{
    static_cast<char>(OpType::DELETE), 0, 0, 0, 0, 0, 0};
constexpr static auto remove_tuple_size = 7;

// Update tuple
constexpr static auto update_tuple = std::array<char, BUFFER_WIDTH>{
    static_cast<char>(OpType::UPDATE), 0, 0, 0, 0, 0, 0};
constexpr static auto update_tuple_size = 7;

// Promote tuple
constexpr static auto promote_tuple = std::array<char, BUFFER_WIDTH>{
    static_cast<char>(OpType::PROMOTE), 0, 0, 0, 0, 0, 0};
constexpr static auto promove_tuple_size = 7;

// Yield read
constexpr static auto yield_read = std::array<char, BUFFER_WIDTH>{
    static_cast<char>(OpType::YIELD_READ), 0, 0, 0, 0, 0, 0};
constexpr static auto yield_read_size = 7;

// Initializing database
static __attribute__((no_destroy)) auto database = Database();

int main() {
  // Create server socket must be UDP
  int server_fd = socket(AF_UNIX, SOCK_DGRAM, 0);
  if (-1 == server_fd) {
    std::cerr << "Failed to create server socket: " << strerror(errno)
              << std::endl;
    std::exit(EXIT_FAILURE);
    return 0;
  }
  struct sockaddr_un server_addr;
  server_addr.sun_family = AF_UNIX;
  strncpy(server_addr.sun_path, UNIX_SOCKET_PATH,
          sizeof(server_addr.sun_path) - 1);
  errno = 0;
  unlink(UNIX_SOCKET_PATH);
  if (errno != 0 && errno != ENOENT) {
    std::cerr << "Failed to unlink existing socket: " << strerror(errno)
              << std::endl;
    std::exit(EXIT_FAILURE);
    return 0;
  }
  if (-1 == bind(server_fd, reinterpret_cast<struct sockaddr*>(&server_addr),
                 sizeof(server_addr))) {
    std::cerr << "Failed to bind server socket: " << strerror(errno)
              << std::endl;
    std::exit(EXIT_FAILURE);
    return 0;
  }
  auto manager_thread = std::thread([&]() {
    ThreadManager manager(&database, server_fd);
    manager.start();
  })
  ;

  return 0;
}

TEST_CASE("MainThread Transaction Start #1") {
  Database db;
  ThreadManager main_thread(db);

  std::thread server_thread([&]() { main_thread.start(); });

  TestClient client;
  std::array<char, BUFFER_WIDTH> buffer{};
  Parser parser(buffer, 0);
  parser.set_op(OpType::START_TXN);
  parser.set_buffer_size(7);

  CHECK(client.send_request(buffer, parser.get_buffer_size()));

  std::array<char, BUFFER_WIDTH> response{};
  CHECK(client.receive_response(response));

  Parser response_parser(response, BUFFER_WIDTH);
  auto ack = response_parser.get_ack();
  CHECK(ack);
  CHECK(ack.unwrap() == OpAck::SUCCESS);

  main_thread.stop();
  server_thread.join();
}

}  // namespace PawnDB
