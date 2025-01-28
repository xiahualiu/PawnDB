#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include "pawndb/schema/demo.h"
#include "pawndb/types/thread_manager.h"

using namespace PawnDB;

int main() {
  auto thread_manager = ThreadManager(&Database::get_db_instance());
  auto thread_manager_ptr = &thread_manager;

  // Start main thread in a separate thread
  auto thread = std::thread([&]() { thread_manager_ptr->start(); });

  std::this_thread::sleep_for(std::chrono::seconds(60));

  // Stop main thread
  thread_manager.stop();
  thread.join();

  return 0;
}
