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

  // Create 20 transactions
  for (int i = 0; i < 20; i++) {
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
    CHECK(txn_id_recv.unwrap() == i);
    auto ack = response_parser.get_ack();
    CHECK(ack);
    CHECK(ack.unwrap() == OpAck::SUCCESS);
  }

  // The buffer number used should be 1 when idle, only the thread manager
  // uses 1 for recvfrom function.
  CHECK(Database::get_db_instance().buffers_.size() == 1);

  // Each txn add 1 tuple
  for (int i = 0; i < 20; i++) {
    buffer_t buffer;
    Parser parser_in(buffer, BUFFER_WIDTH);

    // Prepare the START TXN request
    parser_in.set_op(OpType::ADD_TUPLE);
    parser_in.set_op_id(2);
    parser_in.set_tbl(1);
    parser_in.set_txn(static_cast<txn_id_t>(i));
    auto new_tuple =
        StudentTuple{"John Doe", static_cast<std::uint8_t>(20 + i), 0};
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
    CHECK(txn_id_recv.unwrap() == i);
    auto ack = response_parser.get_ack();
    CHECK(ack);
    CHECK(ack.unwrap() == OpAck::SUCCESS);
  }

  // The buffer number used should be 21.
  CHECK(Database::get_db_instance().buffers_.size() == 21);

  // Commit all the txns
  for (int i = 0; i < 20; i++) {
    buffer_t buffer;
    Parser parser_in(buffer, BUFFER_WIDTH);

    // Prepare the START TXN request
    parser_in.set_op(OpType::COMMIT_TXN);
    parser_in.set_op_id(3);
    parser_in.set_txn(static_cast<txn_id_t>(i));
    CHECK(7 == client.send_request(buffer, 7));
    // No response because half of them are blocking on commit
  }

  // Ensure all txns are committed
  std::this_thread::sleep_for(std::chrono::seconds(1));

  // Check student table size
  CHECK(Database::get_db_instance().students_.size() == StudentTable::Rows);

  // Check buffer size after commit, should be 21, 10 (commit_request_job) + 10
  // (stored commit buffer)+ 1 (manager buffer)
  CHECK(Database::get_db_instance().buffers_.size() == 21);

  // There should be exactly 10 acks, 10 blocking.
  for (int i = 0; i < 10; i++) {
    buffer_t buffer;

    CHECK(7 == client.receive_response(buffer));
    Parser response_parser(buffer, 7);
    auto op_recv = response_parser.get_op();
    CHECK(op_recv);
    CHECK(op_recv.unwrap() == OpType::COMMIT_TXN);
    auto ack_r = response_parser.get_ack();
    CHECK(ack_r);
    CHECK(ack_r.unwrap() == OpAck::SUCCESS);
  }

  // Create another 10 workers, each worker remove 1 tuple
  for (int i = 0; i < 10; i++) {
    buffer_t buffer;
    Parser parser_in(buffer, BUFFER_WIDTH);

    // Prepare the START TXN request
    parser_in.set_op(OpType::START_TXN);
    parser_in.set_op_id(static_cast<op_t>(i));
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
    CHECK(op_id_recv.unwrap() == i);
    CHECK(response_parser.get_txn().unwrap() == 20 + i);
    auto ack = response_parser.get_ack();
    CHECK(ack);
    CHECK(ack.unwrap() == OpAck::SUCCESS);
  }

  // Check buffer size after commit, should be 21, 10 (commit_request_job) + 10
  // (stored commit buffer)+ 1 (manager buffer)
  CHECK(Database::get_db_instance().buffers_.size() == 21);

  // Each worker remove 1 tuple
  for (int i = 0; i < 10; i++) {
    buffer_t buffer;
    Parser parser_in(buffer, BUFFER_WIDTH);

    CHECK(Database::get_db_instance().students_._test_s_avail_cnt() == 10);
    CHECK(Database::get_db_instance().students_._test_x_avail_cnt() == 10);

    // Acquire exclusive lock on the tuple
    parser_in.set_op(OpType::EXCLUSIVE_READ);
    parser_in.set_op_id(static_cast<op_t>(i));
    parser_in.set_tbl(1);
    parser_in.set_txn(static_cast<txn_id_t>(20 + i));
    CHECK(46 == client.send_request(buffer, 46));

    // Check response
    auto recv_size = client.receive_response(buffer);
    CHECK(recv_size == 46);
    Parser response_parser(buffer, recv_size);
    auto op_recv = response_parser.get_op();
    CHECK(op_recv);
    CHECK(op_recv.unwrap() == OpType::EXCLUSIVE_READ);
    auto op_id_recv = response_parser.get_op_id();
    CHECK(op_id_recv);
    CHECK(op_id_recv.unwrap() == i);
    auto txn_id_recv = response_parser.get_txn();
    CHECK(txn_id_recv);
    CHECK(txn_id_recv.unwrap() == 20 + i);
    auto tuple_key = response_parser.get_key().unwrap();
    auto ack = response_parser.get_ack();
    CHECK(ack);
    CHECK(ack.unwrap() == OpAck::SUCCESS);

    // Prepare the DELETE request
    parser_in.set_op(OpType::DELETE);
    parser_in.set_op_id(static_cast<op_t>(i));
    parser_in.set_tbl(1);
    parser_in.set_txn(static_cast<txn_id_t>(20 + i));
    parser_in.set_key(tuple_key);
    CHECK(9 == client.send_request(buffer, 9));

    // Check response
    recv_size = client.receive_response(buffer);
    CHECK(recv_size == 9);
    Parser response_parser_2 = Parser(buffer, recv_size);
    auto op_recv_2 = response_parser_2.get_op();
    CHECK(op_recv_2);
    CHECK(op_recv_2.unwrap() == OpType::DELETE);
    auto op_id_recv_2 = response_parser_2.get_op_id();
    CHECK(op_id_recv_2);
    CHECK(op_id_recv_2.unwrap() == i);
    auto txn_id_recv_2 = response_parser_2.get_txn();
    CHECK(txn_id_recv_2);
    CHECK(txn_id_recv_2.unwrap() == 20 + i);
    auto ack_2 = response_parser_2.get_ack();
    CHECK(ack_2);
    CHECK(ack_2.unwrap() == OpAck::SUCCESS);

    // Prepare Commit Request
    parser_in.set_op(OpType::COMMIT_TXN);
    parser_in.set_op_id(static_cast<op_t>(i));
    parser_in.set_txn(static_cast<txn_id_t>(20 + i));
    CHECK(7 == client.send_request(buffer, 7));

    // We should be able to recv 2 acks, one from the blocking worker and one
    // for this commit
    recv_size = client.receive_response(buffer);
    recv_size = client.receive_response(buffer);
  }

  // Ensure all txns are committed
  std::this_thread::sleep_for(std::chrono::seconds(1));
  // Check student table size, should still be 10, because we just remove 10
  // tuples and the blocking workers will insert another 10 tuples.
  CHECK(Database::get_db_instance().students_.size() == StudentTable::Rows);
  // Check buffer size after commit, should be 1, only the thread manager uses 1
  // for recvfrom function.
  CHECK(Database::get_db_instance().buffers_.size() == 1);
}
