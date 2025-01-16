/**
 * @file main.cpp
 * @brief Parser unit test
 * @version 0.1
 * @date 2025-01-04
 *
 * @copyright Copyright (c) 2025
 *
 */

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include "pawndb/traits/parser.h"
#include "doctest/doctest.h"
#include "pawndb/params.h"
#include "pawndb/types/buffer_table.h"
#include "pawndb/types/parser.h"

namespace PawnDB {

TEST_CASE("Parser Basic Operation #1") {
  BufferTable table;
  auto ref_r = table.request();
  auto buffer = ref_r.unwrap();
  auto& buf = buffer.buffer();

  buf = {static_cast<char>(OpType::START_TXN), 1, 2, 0, 0, 0, 0, 3, 4, 0, 0, 0, 0};

  Parser parser(buffer, 11);

  auto op = parser.get_op();
  CHECK(op);
  CHECK(op.unwrap() == OpType::START_TXN);
  auto op_id = parser.get_op_id();
  CHECK(op_id);
  CHECK(op_id.unwrap() == 1);
  auto txn = parser.get_txn();
  CHECK(txn);
  CHECK(txn.unwrap() == 2);
  auto tbl = parser.get_tbl();
  CHECK(tbl);
  CHECK(tbl.unwrap() == 3);
  auto tp = parser.get_key();
  CHECK(tp);
  CHECK(tp.unwrap() == 4);
}

TEST_CASE("Parser Basic Operation #2") {
  BufferTable table;
  auto ref_r = table.request();
  auto buffer = ref_r.unwrap();
  auto& buf = buffer.buffer();
  Parser parser(buffer, 13);
  parser.set_ack(OpAck::SUCCESS);
  CHECK(buf[6] == static_cast<char>(OpAck::SUCCESS));
  parser.set_txn_id(std::uint16_t(45678));
  CHECK(*reinterpret_cast<txn_id_t*>(&buf[2]) == 45678);
  parser.set_key(127);
  CHECK(buf[8] == 127);
}

TEST_CASE("Parser Basic Operation #3") {
  BufferTable table;
  auto ref_r = table.request();
  auto buffer = ref_r.unwrap();
  Parser parser(buffer, 13);
  parser.set_buffer_size(20);
  CHECK(parser.get_buffer_size() == 20);
  CHECK(parser.get_tuple_offset() == 9);
}

TEST_CASE("Parser Bad Operation #1") {
  BufferTable table;
  auto ref_r = table.request();
  auto buffer = ref_r.unwrap();
  Parser parser(buffer, 0);
  auto op = parser.get_op();
  CHECK(!op);
  CHECK(op.getError() == ParserError::ReadAfterEnd);
  auto op_id = parser.get_op_id();
  CHECK(!op_id);
  CHECK(op_id.getError() == ParserError::ReadAfterEnd);
  auto txn = parser.get_txn();
  CHECK(!txn);
  CHECK(txn.getError() == ParserError::ReadAfterEnd);
  auto tbl = parser.get_tbl();
  CHECK(!tbl);
  CHECK(tbl.getError() == ParserError::ReadAfterEnd);
  auto tp = parser.get_key();
  CHECK(!tp);
  CHECK(tp.getError() == ParserError::ReadAfterEnd);
}

TEST_CASE("Parser Bad Operation #2") {
  BufferTable table;
  auto ref_r = table.request();
  auto buffer = ref_r.unwrap();
  auto& buf = buffer.buffer();
  buf[0] = static_cast<char>(OpType::MAX_OP_VALUE);
  Parser parser(buffer, 11);
  auto op = parser.get_op();
  CHECK(!op);
  CHECK(op.getError() == ParserError::InvalidValue);
}

}  // namespace PawnDB
