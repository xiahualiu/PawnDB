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

TEST_CASE("BufferTable Basic Operations") {
  BufferTable table;
  CHECK(table.empty());
  CHECK(!table.full());
  CHECK(table.size() == 0);
}

TEST_CASE("BufferTable Full") {
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

TEST_CASE("BufferTable Release") {
  BufferTable table;

  // Request and release
  auto ref_r = table.request();
  CHECK(ref_r);
  CHECK(table.size() == 1);

  ref_r.unwrap().release();
  CHECK(table.empty());
  CHECK(table.size() == 0);
}

TEST_CASE("BufferTable Next Fit") {
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

TEST_CASE("BufferRef Construction") {
  BufferRef ref;  // Default constructor
  CHECK(ref.null());
}

TEST_CASE("BufferRef Copy Operations") {
  BufferTable table;
  auto ref1_r = table.request();
  CHECK(ref1_r);
  CHECK(ref1_r.unwrap()._test_index() == 0);

  // Copy constructor
  BufferRef ref2(ref1_r.unwrap());
  CHECK(!ref2.null());
  CHECK(table.size() == 1);
  CHECK(ref2._test_index() == 0);

  // Copy assignment
  BufferRef ref3;
  ref3 = ref1_r.unwrap();
  CHECK(!ref3.null());
  CHECK(table.size() == 1);
  CHECK(ref3._test_index() == 0);
}

TEST_CASE("BufferRef Buffer Access") {
  BufferTable table;
  auto ref_r = table.request();
  CHECK(ref_r);

  // Access buffer
  auto& buf = ref_r.unwrap().buffer();
  buf[0] = 'x';
  CHECK(buf[0] == 'x');

  // Release should invalidate
  ref_r.unwrap().release();
}

}  // namespace PawnDB
