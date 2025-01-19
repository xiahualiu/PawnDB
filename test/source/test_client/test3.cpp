#define DOCTEST_CONFIG_IMPLEMENT

#include <sys/socket.h>
#include <sys/un.h>

#include <thread>

#include "doctest/doctest.h"
#include "pawndb/params.h"
#include "pawndb/schema/demo.h"
#include "pawndb/traits/parser.h"
#include "pawndb/types/parser.h"
#include "pawndb/types/student_table.h"
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
  // Create a client
  auto client = TestClient();

  // Check student table size
  CHECK(Database::get_db_instance().students_.size() == 0);

  // Start 1 txn
  {
    buffer_t buffer;
    Parser parser_in(buffer, BUFFER_WIDTH);

    // Prepare the START TXN request
    parser_in.set_op(OpType::START_TXN);
    parser_in.set_op_id(1);
    CHECK(7 == client.send_request(buffer, 7));

    // Check response
    auto recv_size = client.receive_response(buffer);
    CHECK(recv_size == 7);
    Parser response_parser(buffer, recv_size);
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

  // The buffer number used should be 1 when idle, only the thread manager uses
  // 1 for recvfrom function.
  CHECK(Database::get_db_instance().buffers_.size() == 1);

  {
    buffer_t buffer;
    Parser parser_in(buffer, BUFFER_WIDTH);

    // Prepare the START TXN request
    parser_in.set_op(OpType::ADD_TUPLE);
    parser_in.set_op_id(2);
    parser_in.set_tbl(1);
    parser_in.set_txn(0);
    auto new_tuple = StudentTuple{"John Doe", 20, 0};
    new_tuple.set_checksum();
    auto write_size = new_tuple.serialize(buffer, parser_in.get_tuple_offset());
    CHECK(write_size);
    CHECK(write_size.unwrap() == 37);
    // Should be 9 bytes + 37 bytes tuple serialized size
    CHECK(46 == client.send_request(buffer, parser_in.get_tuple_offset() +
                                                write_size.unwrap()));

    // Check response
    auto recv_size = client.receive_response(buffer);
    CHECK(recv_size == 46);
    Parser response_parser(buffer, recv_size);
    auto op_recv = response_parser.get_op();
    CHECK(op_recv);
    CHECK(op_recv.unwrap() == OpType::ADD_TUPLE);
    auto op_id_recv = response_parser.get_op_id();
    CHECK(op_id_recv);
    CHECK(op_id_recv.unwrap() == 2);
    auto txn_id_recv = response_parser.get_txn();
    CHECK(txn_id_recv);
    CHECK(txn_id_recv.unwrap() == 0);
    auto ack = response_parser.get_ack();
    CHECK(ack);
    CHECK(ack.unwrap() == OpAck::SUCCESS);
  }

  // The buffer number used should be 2, because we added a new tuple it is
  // stored in the buffer
  CHECK(Database::get_db_instance().buffers_.size() == 2);

  // Commit the txn
  {
    buffer_t buffer;
    Parser parser_in(buffer, BUFFER_WIDTH);

    // Prepare the START TXN request
    parser_in.set_op(OpType::COMMIT_TXN);
    parser_in.set_op_id(3);
    parser_in.set_txn(0);
    CHECK(7 == client.send_request(buffer, 7));

    // Check response
    auto recv_size = client.receive_response(buffer);
    CHECK(recv_size == 7);
    Parser response_parser(buffer, recv_size);
    auto op_recv = response_parser.get_op();
    CHECK(op_recv);
    CHECK(op_recv.unwrap() == OpType::COMMIT_TXN);
    auto op_id_recv = response_parser.get_op_id();
    CHECK(op_id_recv);
    CHECK(op_id_recv.unwrap() == 3);
    auto txn_id_recv = response_parser.get_txn();
    CHECK(txn_id_recv);
    CHECK(txn_id_recv.unwrap() == 0);
    auto ack = response_parser.get_ack();
    CHECK(ack);
    CHECK(ack.unwrap() == OpAck::SUCCESS);
  }

  // Ensure the worker has done commit
  std::this_thread::sleep_for(std::chrono::seconds(1));

  // Check student table size
  CHECK(Database::get_db_instance().students_.size() == 1);

  // Check buffer size after commit, should be 1
  CHECK(Database::get_db_instance().buffers_.size() == 1);
}
