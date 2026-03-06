#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include <utility>

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
  refs.clear();
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
  refs.pop_back();
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

  ref_r.unwrap() = BufferRef{};
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
  ref1_r.unwrap() = BufferRef{};

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

TEST_CASE("BufferRef Reference Count #1") {
  BufferTable table;
  auto ref1_r = table.request();
  CHECK(ref1_r);
  CHECK(table._test_ref_count(0) == 1);

  BufferRef ref2(ref1_r.unwrap());
  CHECK(table._test_ref_count(0) == 2);
  CHECK(table.size() == 1);

  ref2 = BufferRef{};
  CHECK(table._test_ref_count(0) == 1);
  CHECK(table.size() == 1);

  ref1_r.unwrap() = BufferRef{};
  CHECK(table._test_ref_count(0) == 0);
  CHECK(table.size() == 0);
}

TEST_CASE("BufferRef Buffer Access #1") {
  BufferTable table;
  auto ref_r = table.request();
  CHECK(ref_r);
  auto& buf = ref_r.unwrap().buffer();
  buf[0] = 'x';
  CHECK(buf[0] == 'x');
  ref_r.unwrap() = BufferRef{};
}

TEST_CASE("BufferRef Clone #1") {
  BufferTable table;
  auto ref1_r = table.request();
  CHECK(ref1_r);
  CHECK(ref1_r.unwrap()._test_index() == 0);
  auto ref2_r = table.request();
  CHECK(ref2_r);
  CHECK(ref2_r.unwrap()._test_index() == 1);

  // Copy value
  auto ref3 = ref2_r.unwrap().copy();
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
  ref3.copy_from(ref2_r.unwrap());
  CHECK(ref3._test_index() == 1);
  CHECK(table.size() == 2);
  CHECK(table._test_is_used(1) == true);
}

// --- new move tests ---
TEST_CASE("BufferRef Move Operations #1") {
  BufferTable table;
  auto ref1_r = table.request();
  CHECK(ref1_r);

  // normal move constructor
  BufferRef ref1 = ref1_r.unwrap();
  BufferRef ref2(std::move(ref1));
  CHECK(ref1._test_null());
  CHECK(!ref2._test_null());
  CHECK(table.size() == 1);

  // move assignment
  BufferRef ref3;
  ref3 = std::move(ref2);
  CHECK(ref2._test_null());
  CHECK(!ref3._test_null());
  CHECK(table.size() == 1);

  // trait_move
  auto ref4 = ref3.move();
  CHECK(ref3._test_null());
  CHECK(!ref4._test_null());
  CHECK(table.size() == 1);

  // move_from via trait
  ref3.move_from(std::move(ref4));
  CHECK(ref4._test_null());
  CHECK(!ref3._test_null());
}

}  // namespace PawnDB
