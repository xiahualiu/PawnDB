#ifndef PAWNDB_TEST_CLIENT_H
#define PAWNDB_TEST_CLIENT_H

#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include <cstring>
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

  bool send_request(const buffer_t& buffer, std::size_t size) noexcept {
    auto sent = sendto(client_fd_, buffer.data(), size, 0,
                       reinterpret_cast<struct sockaddr*>(&server_addr_),
                       sizeof(server_addr_));
    if (sent == -1) {
      std::cerr << "Failed to send request: " << strerror(errno) << std::endl;
      return false;
    }
    return true;
  }

  bool receive_response(buffer_t& response) noexcept {
    socklen_t server_len = sizeof(server_addr_);
    auto received = recvfrom(client_fd_, response.data(), response.size(), 0,
                             reinterpret_cast<struct sockaddr*>(&server_addr_),
                             &server_len);
    return received != -1;
  }
};

}

#endif  // PAWNDB_TEST_CLIENT_H
