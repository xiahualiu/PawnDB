#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include "doctest/doctest.h"
#include "pawndb/params.h"
#include "pawndb/types/commit_table.h"

namespace PawnDB {

TEST_CASE("commit_table Basic Operations #1") {
  commit_table table;
  CHECK(table.empty());
  CHECK(!table.full());
  CHECK(table.size() == 0);
}

TEST_CASE("commit_table Queue Operations #1") {
  commit_table table;
  buf_table buffers;
  auto buffer_r = buffers.request();
  CHECK(buffer_r);

  // Add commit entry
  commit_entry entry({1, 2}, OpType::ADD_TUPLE, buffer_r.unwrap());
  CHECK(table.send(entry) == QueueError::None);
  CHECK(!table.empty());
  CHECK(table.size() == 1);

  // Get entry
  auto get_r = table.get();
  CHECK(get_r);
  auto& commit = get_r.unwrap();
  auto [tbl, tp] = commit.key().disassemble();
  CHECK(tbl == 1);
  CHECK(tp == 2);
  CHECK(commit.op() == OpType::ADD_TUPLE);

  // Remove entry
  table.pop();
  CHECK(table.empty());
  CHECK(table.size() == 0);
}

TEST_CASE("commit_table Full #1") {
  commit_table table;
  buf_table buffers;

  // Fill up to capacity
  for (tbl_row_t i = 0; i < MAX_COMMIT_PER_TRANSACTION; i++) {
    auto buffer_r = buffers.request();
    CHECK(buffer_r);
    commit_entry entry({1, i}, OpType::ADD_TUPLE, buffer_r.unwrap());
    CHECK(table.send(entry) == QueueError::None);
  }

  CHECK(table.full());
  CHECK(table.size() == MAX_COMMIT_PER_TRANSACTION);

  // Should fail when full
  auto buffer_r = buffers.request();
  CHECK(buffer_r);
  commit_entry entry({1, MAX_COMMIT_PER_TRANSACTION}, OpType::ADD_TUPLE,
                     buffer_r.unwrap());
  CHECK(table.send(entry) == QueueError::Full);
}

TEST_CASE("commit_table Clear #1") {
  commit_table table;
  buf_table buffers;

  // Add some entries
  for (tbl_row_t i = 0; i < 3; i++) {
    auto buffer_r = buffers.request();
    CHECK(buffer_r);
    commit_entry entry({1, i}, OpType::ADD_TUPLE, buffer_r.unwrap());
    CHECK(table.send(entry) == QueueError::None);
  }

  CHECK(!table.empty());
  CHECK(table.size() == 3);

  // Clear table
  table.clear();
  CHECK(table.empty());
  CHECK(table.size() == 0);
}

TEST_CASE("commit_entry Copy #1") {
  commit_table table;
  buf_table buffers;
  auto buffer_r = buffers.request();
  CHECK(buffer_r);

  // Add commit entry
  commit_entry entry({1, 2}, OpType::ADD_TUPLE, buffer_r.unwrap());
  CHECK(table.send(entry) == QueueError::None);

  // Copy entry
  commit_entry copy_entry;
  copy_entry.copy_from(entry);
  CHECK(copy_entry.key() == entry.key());
  CHECK(copy_entry.op() == entry.op());
}

TEST_CASE("commit_entry Clone #1") {
  commit_table table;
  buf_table buffers;
  auto buffer_r = buffers.request();
  CHECK(buffer_r);

  // Add commit entry
  commit_entry entry({1, 2}, OpType::ADD_TUPLE, buffer_r.unwrap());
  CHECK(table.send(entry) == QueueError::None);

  // Copy value
  auto clone_entry = entry.copy();
  CHECK(clone_entry.key() == entry.key());
  CHECK(clone_entry.op() == entry.op());
}

TEST_CASE("commit_entry Buffer #1") {
  commit_table table;
  buf_table buffers;
  auto buffer_r = buffers.request();
  CHECK(buffer_r);

  // Add commit entry
  commit_entry entry({1, 2}, OpType::ADD_TUPLE, buffer_r.unwrap());
  CHECK(table.send(entry) == QueueError::None);

  // Check buffer
  auto get_r = table.get();
  CHECK(get_r);
  auto& commit = get_r.unwrap();
  CHECK(commit.buf()._test_index() == buffer_r.unwrap()._test_index());
}

}  // namespace PawnDB
