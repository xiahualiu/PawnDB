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

#include "pawndb/main_thread.h"
#include "pawndb/schema/demo.h"

using namespace PawnDB;

static __attribute__((no_destroy)) auto database = Database();

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

  // Start main thread in a separate thread
  auto main_thread = std::thread{[&]() {
    auto _main_thread = MainThread(&database, server_fd);
    _main_thread.start();
  }};

  // Sleep for a while and then clean up
  std::this_thread::sleep_for(std::chrono::seconds(60));
  std::cout << "Shutting down server..." << std::endl;

  // Kill main thread by closing the socket
  shutdown(server_fd, SHUT_RDWR);
  close(server_fd);
  unlink(UNIX_SOCKET_PATH);

  main_thread.join();  // Wait for main thread to finish
  return 0;
}
