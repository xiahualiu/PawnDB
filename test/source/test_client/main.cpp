/**
 * @file main.cpp
 * @brief Channal unit tests
 * @version 0.1
 * @date 2025-01-04
 *
 * @copyright Copyright (c) 2025
 *
 */
#include <iostream>
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include <sys/socket.h>
#include <sys/un.h>

#include <thread>

#include "doctest/doctest.h"
#include "pawndb/params.h"
#include "pawndb/schema/demo.h"
#include "pawndb/traits/parser.h"
#include "pawndb/types/parser.h"
#include "pawndb/types/thread_manager.h"

namespace PawnDB {

class TestClient {
 private:
  int client_fd_;
  struct sockaddr_un server_addr_;

  const char* CLIENT_PATH = "/tmp/test-client.sock";

 public:
  TestClient() {
    // Create UDP socket
    client_fd_ = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (client_fd_ == -1) {
      throw std::runtime_error("Failed to create socket");
    }

    // Setup server address
    memset(server_addr_.sun_path, 0, sizeof(server_addr_.sun_path));
    server_addr_.sun_family = AF_UNIX;
    strncpy(server_addr_.sun_path, UNIX_SOCKET_PATH,
            sizeof(server_addr_.sun_path) - 1);

    // Setup client address
    struct sockaddr_un client_addr;
    memset(&client_addr, 0, sizeof(client_addr));
    client_addr.sun_family = AF_UNIX;
    strncpy(client_addr.sun_path, CLIENT_PATH,
            sizeof(client_addr.sun_path) - 1);
    unlink(CLIENT_PATH);

    // Bind client address
    if (bind(client_fd_, reinterpret_cast<struct sockaddr*>(&client_addr),
             sizeof(client_addr))) {
      throw std::runtime_error("Failed to bind client address");
    }
  }

  ~TestClient() noexcept {
    if (client_fd_ != -1) {
      close(client_fd_);
    }
  }

  // Non-copyable
  TestClient(const TestClient&) = delete;
  TestClient& operator=(const TestClient&) = delete;

  bool send_request(const buffer_t& buffer, std::size_t size) noexcept {
    auto sent = sendto(client_fd_, buffer.data(), size, 0,
                       reinterpret_cast<struct sockaddr*>(&server_addr_),
                       sizeof(server_addr_));
    return sent != -1;
  }

  bool receive_response(buffer_t& response) noexcept {
    socklen_t server_len = sizeof(server_addr_);
    auto received = recvfrom(client_fd_, response.data(), response.size(), 0,
                             reinterpret_cast<struct sockaddr*>(&server_addr_),
                             &server_len);
    return received != -1;
  }
};

TEST_CASE("MainThread Transaction Start #1") {
  // Create server socket must be UDP
  int server_fd = socket(AF_UNIX, SOCK_DGRAM, 0);
  CHECK(-1 != server_fd);
  struct sockaddr_un server_addr;
  server_addr.sun_family = AF_UNIX;
  strncpy(server_addr.sun_path, UNIX_SOCKET_PATH,
          sizeof(server_addr.sun_path) - 1);
  unlink(UNIX_SOCKET_PATH);
  CHECK(-1 != bind(server_fd, reinterpret_cast<struct sockaddr*>(&server_addr),
                   sizeof(server_addr)));
  // Set receive timeout to 3 seconds
  struct timeval tv;
  tv.tv_sec = 3;  // 3 second timeout
  tv.tv_usec = 0;
  CHECK(-1 !=
        setsockopt(server_fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0);

  ThreadManager manager(&Database::get_instance(), server_fd);
  auto* manager_ptr = &manager;

  auto manager_thread = std::thread([&]() { manager_ptr->start(); });

  auto client = TestClient();

  buffer_t buffer;
  Parser parser_in(buffer, BUFFER_WIDTH);
  parser_in.set_op(OpType::START_TXN);
  parser_in.set_op_id(1);

  CHECK(client.send_request(buffer, 7));

  buffer_t response{};
  CHECK(client.receive_response(response));

  std::cout << "Receive response: " << std::endl;
  Parser response_parser(response, BUFFER_WIDTH);
  auto op_recv = response_parser.get_op();
  CHECK(op_recv);
  CHECK(op_recv.unwrap() == OpType::START_TXN);
  auto op_id_recv = response_parser.get_op_id();
  CHECK(op_id_recv);
  CHECK(op_id_recv.unwrap() == 1);
  auto txn_id_recv = response_parser.get_txn();
  CHECK(txn_id_recv);
  CHECK(txn_id_recv.unwrap() == 0);
  auto ack = response_parser.get_ack();
  CHECK(ack);
  CHECK(ack.unwrap() == OpAck::SUCCESS);

  manager.stop();
  manager_thread.join();
}

}  // namespace PawnDB
