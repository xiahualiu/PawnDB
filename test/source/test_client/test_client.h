#ifndef PAWNDB_TEST_CLIENT_H
#define PAWNDB_TEST_CLIENT_H

#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include <cstring>
#include <iomanip>
#include <iostream>
#include <stdexcept>

#include "pawndb/params.h"

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
      unlink(CLIENT_PATH);
    }
  }

  // Non-copyable
  TestClient(const TestClient&) = delete;
  TestClient& operator=(const TestClient&) = delete;

  std::size_t send_request(const buffer_t& buffer, std::size_t size) {
    auto sent = sendto(client_fd_, buffer.data(), size, 0,
                       reinterpret_cast<struct sockaddr*>(&server_addr_),
                       sizeof(server_addr_));
    if (sent <= 0) {
      throw std::runtime_error("Failed to send request.");
    }
    return static_cast<std::size_t>(sent);
  }

  std::size_t receive_response(buffer_t& response) {
    socklen_t server_len = sizeof(server_addr_);
    auto received = recvfrom(client_fd_, response.data(), response.size(), 0,
                             reinterpret_cast<struct sockaddr*>(&server_addr_),
                             &server_len);
    if (received <= 0) {
      throw std::runtime_error("Failed to receive response.");
    }
    return static_cast<std::size_t>(received);
  }

  inline static void hex_dump(const char* buffer, std::size_t size) {
    for (std::size_t i = 0; i < size; i += 16) {
      // Print offset
      std::cout << "\033[33m" << std::setfill('0') << std::setw(4) << std::hex
                << i << "\033[0m  ";

      // Print hex values
      for (std::size_t j = 0; j < 16; j++) {
        if (i + j < size) {
          std::cout << std::setfill('0') << std::setw(2) << std::hex
                    << (static_cast<int>(buffer[i + j]) & 0xFF) << " ";
        } else {
          std::cout << "   ";
        }
        if (j == 7) std::cout << " ";
      }

      // Print ASCII
      std::cout << " \033[36m|";
      for (std::size_t j = 0; j < 16; j++) {
        if (i + j < size) {
          char c = buffer[i + j];
          std::cout << (std::isprint(c) ? c : '.');
        }
      }
      std::cout << "|\033[0m\n";
    }
    std::cout << std::dec;  // Reset to decimal
  }
};

}  // namespace PawnDB

#endif  // PAWNDB_TEST_CLIENT_H
