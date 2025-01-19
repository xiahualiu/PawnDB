/**
 * @file main.cpp
 * @brief Buffer table unit test
 * @version 0.1
 * @date 2025-01-04
 *
 * @copyright Copyright (c) 2025
 *
 */

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include "doctest/doctest.h"
#include "pawndb/types/buffer_table.h"

namespace PawnDB {

TEST_CASE("BufferTable Basic Operations #1") {
  BufferTable table;
  CHECK(table.empty());
  CHECK(!table.full());
  CHECK(table.size() == 0);
  CHECK(table._test_is_used(0) == false);
}

TEST_CASE("BufferTable Full #1") {
  BufferTable table;
  std::vector<BufferRef> refs;

  // Fill up to capacity
  for (size_t i = 0; i < BUFFER_ROWS; i++) {
    auto ref_r = table.request();
    CHECK(ref_r);
    refs.push_back(ref_r.unwrap());
  }

  // Check all buffers are used
  for (size_t i = 0; i < BUFFER_ROWS; i++) {
    CHECK(table._test_is_used(i) == true);
  }

  CHECK(table.full());
  CHECK(table.size() == BUFFER_ROWS);

  // Should fail when full
  auto ref_r = table.request();
  CHECK(!ref_r);
  CHECK(ref_r.getError() == BufferError::Full);

  // Release all buffers
  for (auto& ref : refs) {
    ref.release();
  }
  CHECK(table.empty());
}

TEST_CASE("BufferTable Full #2") {
  BufferTable table;
  std::vector<BufferRef> refs;

  // Fill up to capacity
  for (size_t i = 0; i < BUFFER_ROWS; i++) {
    auto ref_r = table.request();
    CHECK(ref_r);
    refs.push_back(ref_r.unwrap());
  }

  CHECK(table.full());
  CHECK(table.size() == BUFFER_ROWS);

  // Release the last buffer
  refs.back().release();
  CHECK(!table.full());
  CHECK(table._test_is_used(BUFFER_ROWS - 1) == false);

  // Requesting again should succeed
  auto ref_r = table.request();
  CHECK(ref_r);
  CHECK(table.size() == BUFFER_ROWS);
  CHECK(table.full());
  CHECK(table._test_is_used(BUFFER_ROWS - 1) == true);
}

TEST_CASE("BufferTable Full #3") {
  BufferTable table;
  std::vector<BufferRef> refs;

  // Fill up to capacity
  for (size_t i = 0; i < BUFFER_ROWS; i++) {
    auto ref_r = table.request();
    CHECK(ref_r);
    refs.push_back(ref_r.unwrap());
  }

  CHECK(table.full());
  CHECK(table.size() == BUFFER_ROWS);

  // Release all buffers
  table.clear();
  CHECK(!table.full());
  CHECK(table.size() == 0);
  CHECK(table.empty());

  for (size_t i = 0; i < BUFFER_ROWS; i++) {
    CHECK(table._test_is_used(i) == false);
  }
}

TEST_CASE("BufferTable Release #1") {
  BufferTable table;

  // Request and release
  auto ref_r = table.request();
  CHECK(ref_r);
  CHECK(table.size() == 1);

  ref_r.unwrap().release();
  CHECK(table.empty());
  CHECK(table.size() == 0);
}

TEST_CASE("BufferTable Next Fit #1") {
  BufferTable table;

  // First request
  auto ref1_r = table.request();
  CHECK(ref1_r);
  CHECK(ref1_r.unwrap()._test_index() == 0);

  // Release first buffer
  ref1_r.unwrap().release();

  // Next request should be 1
  auto ref2_r = table.request();
  CHECK(ref2_r);
  CHECK(ref2_r.unwrap()._test_index() == 1);
}

TEST_CASE("BufferRef Construction #1") {
  BufferRef ref;  // Default constructor
  CHECK(ref._test_null());
}

TEST_CASE("BufferRef Copy Operations #1") {
  BufferTable table;
  auto ref1_r = table.request();
  CHECK(ref1_r);
  CHECK(ref1_r.unwrap()._test_index() == 0);

  // Copy constructor
  BufferRef ref2(ref1_r.unwrap());
  CHECK(!ref2._test_null());
  CHECK(table.size() == 1);
  CHECK(ref2._test_index() == 0);

  // Copy assignment
  BufferRef ref3;
  ref3 = ref1_r.unwrap();
  CHECK(!ref3._test_null());
  CHECK(table.size() == 1);
  CHECK(ref3._test_index() == 0);
}

TEST_CASE("BufferRef Buffer Access #1") {
  BufferTable table;
  auto ref_r = table.request();
  CHECK(ref_r);
  auto& buf = ref_r.unwrap().buffer();
  buf[0] = 'x';
  CHECK(buf[0] == 'x');
  ref_r.unwrap().release();
}

TEST_CASE("BufferRef Clone #1") {
  BufferTable table;
  auto ref1_r = table.request();
  CHECK(ref1_r);
  CHECK(ref1_r.unwrap()._test_index() == 0);
  auto ref2_r = table.request();
  CHECK(ref2_r);
  CHECK(ref2_r.unwrap()._test_index() == 1);

  // Clone
  auto ref3 = ref2_r.unwrap().clone();
  CHECK(!ref3._test_null());
  CHECK(table.size() == 2);
  CHECK(ref3._test_index() == 1);
}

TEST_CASE("BufferRef Copy #1") {
  BufferTable table;
  auto ref1_r = table.request();
  CHECK(ref1_r);
  CHECK(ref1_r.unwrap()._test_index() == 0);
  auto ref2_r = table.request();
  CHECK(ref2_r);
  CHECK(ref2_r.unwrap()._test_index() == 1);

  // Copy
  BufferRef ref3(ref1_r.unwrap());
  CHECK(!ref3._test_null());
  CHECK(table.size() == 2);
  ref3.copy(ref2_r.unwrap());
  CHECK(ref3._test_index() == 1);
  CHECK(table.size() == 2);
  CHECK(table._test_is_used(1) == true);
}

}  // namespace PawnDB
