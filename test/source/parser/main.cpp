/**
 * @file main.cpp
 * @brief Parser unit test
 * @version 0.1
 * @date 2025-01-04
 *
 * @copyright Copyright (c) 2025
 *
 */

#include "pawndb/params.h"
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include "doctest/doctest.h"
#include "pawndb/parser.h"

namespace PawnDB {

TEST_CASE("Parser Basic Operation #1") {
  std::array<char, BUFFER_WIDTH> buffer = {1, 2, 0, 1, 0, 0, 0,
                                           1, 2, 3, 0, 0, 0};
  Parser parser(buffer, 13);

  auto op = parser.get_op();
  CHECK(op);
  CHECK(op.unwrap() == OpType::START_TXN);
  auto op_id = parser.get_op_id();
  CHECK(op_id);
  CHECK(op_id.unwrap() == 2);
  auto txn = parser.get_txn();
  CHECK(txn);
  CHECK(txn.unwrap() == 1);
  auto tbl = parser.get_tbl();
  CHECK(tbl);
  CHECK(tbl.unwrap() == 1);
  auto tp = parser.get_tp_key();
  CHECK(tp);
  CHECK(tp.unwrap() == 2);
  auto size = parser.get_data_size();
  CHECK(size);
  CHECK(size.unwrap() == 3);
}

TEST_CASE("Parser Basic Operation #2") {
  std::array<char, BUFFER_WIDTH> buffer = {};
  Parser parser(buffer, 13);
  parser.set_ack(OpAck::SUCCESS);
  CHECK(buffer[2] == static_cast<char>(OpAck::SUCCESS));
  parser.set_txn_id(std::uint16_t(45678));
  CHECK(*reinterpret_cast<txn_id_t*>(&buffer[3]) == 45678);
  parser.set_tp_key(127);
  CHECK(buffer[8] == 127);
  parser.set_data_size(std::uint16_t(54321));
  CHECK(*reinterpret_cast<buf_size_t*>(&buffer[9]) == 54321);
}

TEST_CASE("Parser Basic Operation #3") {
  std::array<char, BUFFER_WIDTH> buffer = {};
  Parser parser(buffer, 13);
  parser.set_buffer_size(20);
  CHECK(parser.get_buffer_size() == 20);
  CHECK(parser.buffer().size() == BUFFER_WIDTH);
  CHECK(parser.get_data_offset() == 11);
}

TEST_CASE("Parser Bad Operation #1") {
  std::array<char, BUFFER_WIDTH> buffer = {};
  Parser parser(buffer, 0);
  auto op = parser.get_op();
  CHECK(!op);
  CHECK(op.getError() == ParserError::Bad);
  auto op_id = parser.get_op_id();
  CHECK(!op_id);
  CHECK(op_id.getError() == ParserError::Bad);
  auto txn = parser.get_txn();
  CHECK(!txn);
  CHECK(txn.getError() == ParserError::Bad);
  auto tbl = parser.get_tbl();
  CHECK(!tbl);
  CHECK(tbl.getError() == ParserError::Bad);
  auto tp = parser.get_tp_key();
  CHECK(!tp);
  CHECK(tp.getError() == ParserError::Bad);
  auto size = parser.get_data_size();
  CHECK(!size);
  CHECK(size.getError() == ParserError::Bad);
}

TEST_CASE("Parser Bad Operation #2") {
  std::array<char, BUFFER_WIDTH> buffer = {
      static_cast<char>(OpType::MAX_OP_VALUE), 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  Parser parser(buffer, 11);
  auto op = parser.get_op();
  CHECK(!op);
  CHECK(op.getError() == ParserError::Bad);
}

}  // namespace PawnDB
