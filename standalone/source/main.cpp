/**
 * @file main.cpp
 * @author Xiahua Liu @xiahualiu
 * @brief Demo application for PawnDB.
 * @version 0.1
 * @date 2025-01-03
 *
 * @copyright Copyright (c) 2025
 *
 */

#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include <iostream>

#include "pawndb/schema/demo.h"
#include "pawndb/types/thread_manager.h"

using namespace PawnDB;

int main() {
  // Create server socket must be UDP
  int server_fd = socket(AF_UNIX, SOCK_DGRAM, 0);
  if (-1 == server_fd) {
    std::cerr << "Failed to create server socket: " << strerror(errno)
              << std::endl;
    std::exit(EXIT_FAILURE);
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
  }
  if (-1 == bind(server_fd, reinterpret_cast<struct sockaddr*>(&server_addr),
                 sizeof(server_addr))) {
    std::cerr << "Failed to bind server socket: " << strerror(errno)
              << std::endl;
    std::exit(EXIT_FAILURE);
  }

  // Set receive timeout to 3 seconds
  struct timeval tv;
  tv.tv_sec = 3;  // 3 second timeout
  tv.tv_usec = 0;
  if (setsockopt(server_fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
    std::cerr << "Failed to set socket timeout: " << strerror(errno)
              << std::endl;
    std::exit(EXIT_FAILURE);
  }

  auto thread_manager = ThreadManager(&Database::get_instance(), server_fd);
  auto thread_manager_ptr = &thread_manager;

  // Start main thread in a separate thread
  auto thread = std::thread([&]() { thread_manager_ptr->start(); });

  std::this_thread::sleep_for(std::chrono::seconds(60));

  // Stop main thread
  thread_manager.stop();
  thread.join();

  std::cerr << "Server stopped." << std::endl;
  return 0;
}
