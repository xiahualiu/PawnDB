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
#include "pawndb/schema/demo.h"
#include "pawndb/traits/parser.h"
#include "pawndb/types/parser.h"
#include "pawndb/types/thread_manager.h"
#include "test_client.h"

int main(int argc, char** argv) {

  using namespace PawnDB;
  // Start the thread manager on another thread
  ThreadManager manager(&Database::get_db_instance());
  auto* manager_ptr = &manager;
  auto manager_thread = std::thread([&]() { manager_ptr->start(); });

  doctest::Context context;
  context.applyCommandLine(argc, argv);

  // Will run the tests one by one
  int res = context.run();

  // Stop the thread manager
  manager.stop();
  manager_thread.join();

  return res;  
}

TEST_CASE("MainThread Transaction Start #1") {
  using namespace PawnDB;
  auto client = TestClient();
  buffer_t buffer;
  Parser parser_in(buffer, BUFFER_WIDTH);
  parser_in.set_op(OpType::START_TXN);
  parser_in.set_op_id(1);
  CHECK(client.send_request(buffer, 7));
  buffer_t response{};
  CHECK(client.receive_response(response));
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
}
