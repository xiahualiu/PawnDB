#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include <utility>

#include "doctest/doctest.h"
#include "pawndb/types/buffer_table.h"

namespace PawnDB {

std::uint8_t test_is_used(const buf_table& t, std::size_t i) {
  return t.buffers_[i].ref_count_ > 0;
}

std::uint16_t test_ref_count(const buf_table& t, std::size_t i) {
  return t.buffers_[i].ref_count_;
}

bool test_null(const buf_ref& ref) {
  return ref.table_ == nullptr;
}

std::size_t test_index(const buf_ref& ref) {
  return ref.index_;
}

TEST_CASE("BufferTable Basic Operations #1") {
  buf_table table;
  CHECK(table.empty());
  CHECK(!table.full());
  CHECK(table.size() == 0);
  CHECK(test_is_used(table, 0) == false);
}

TEST_CASE("BufferTable Full #1") {
  buf_table table;
  std::vector<buf_ref> refs;

  // Fill up to capacity
  for (size_t i = 0; i < BUFFER_ROWS; i++) {
    auto ref_r = table.request();
    CHECK(ref_r);
    refs.push_back(ref_r.unwrap());
  }

  // Check all buffers are used
  for (size_t i = 0; i < BUFFER_ROWS; i++) {
    CHECK(test_is_used(table, i) == true);
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
  buf_table table;
  std::vector<buf_ref> refs;

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
  CHECK(test_is_used(table, BUFFER_ROWS - 1) == false);

  // Requesting again should succeed
  auto ref_r = table.request();
  CHECK(ref_r);
  CHECK(table.size() == BUFFER_ROWS);
  CHECK(table.full());
  CHECK(test_is_used(table, BUFFER_ROWS - 1) == true);
}

TEST_CASE("BufferTable Full #3") {
  buf_table table;
  std::vector<buf_ref> refs;

  // Fill up to capacity
  for (size_t i = 0; i < BUFFER_ROWS; i++) {
    auto ref_r = table.request();
    CHECK(ref_r);
    refs.push_back(ref_r.unwrap());
  }

  CHECK(table.full());
  CHECK(table.size() == BUFFER_ROWS);

  // Release all buffers
  table.clear_();
  CHECK(!table.full());
  CHECK(table.size() == 0);
  CHECK(table.empty());

  for (size_t i = 0; i < BUFFER_ROWS; i++) {
    CHECK(test_is_used(table, i) == false);
  }
}

TEST_CASE("BufferTable Release #1") {
  buf_table table;

  // Request and release
  auto ref_r = table.request();
  CHECK(ref_r);
  CHECK(table.size() == 1);

  ref_r.unwrap() = buf_ref{};
  CHECK(table.empty());
  CHECK(table.size() == 0);
}

TEST_CASE("BufferTable Next Fit #1") {
  buf_table table;

  // First request
  auto ref1_r = table.request();
  CHECK(ref1_r);
  CHECK(test_index(ref1_r.unwrap()) == 0);

  // Release first buffer
  ref1_r.unwrap() = buf_ref{};

  // Next request should be 1
  auto ref2_r = table.request();
  CHECK(ref2_r);
  CHECK(test_index(ref2_r.unwrap()) == 1);
}

TEST_CASE("BufferRef Construction #1") {
  buf_ref ref;  // Default constructor
  CHECK(test_null(ref));
}

TEST_CASE("BufferRef Copy Operations #1") {
  buf_table table;
  auto ref1_r = table.request();
  CHECK(ref1_r);
  CHECK(test_index(ref1_r.unwrap()) == 0);

  // Copy constructor
  buf_ref ref2(ref1_r.unwrap());
  CHECK(!test_null(ref2));
  CHECK(table.size() == 1);
  CHECK(test_index(ref2) == 0);

  // Copy assignment
  buf_ref ref3;
  ref3 = ref1_r.unwrap();
  CHECK(!test_null(ref3));
  CHECK(table.size() == 1);
  CHECK(test_index(ref3) == 0);
}

TEST_CASE("BufferRef Reference Count #1") {
  buf_table table;
  auto ref1_r = table.request();
  CHECK(ref1_r);
  CHECK(test_ref_count(table, 0) == 1);

  buf_ref ref2(ref1_r.unwrap());
  CHECK(test_ref_count(table, 0) == 2);
  CHECK(table.size() == 1);

  ref2 = buf_ref{};
  CHECK(test_ref_count(table, 0) == 1);
  CHECK(table.size() == 1);

  ref1_r.unwrap() = buf_ref{};
  CHECK(test_ref_count(table, 0) == 0);
  CHECK(table.size() == 0);
}

TEST_CASE("BufferRef Buffer Access #1") {
  buf_table table;
  auto ref_r = table.request();
  CHECK(ref_r);
  auto& buf = ref_r.unwrap().buffer();
  buf[0] = 'x';
  CHECK(buf[0] == 'x');
  ref_r.unwrap() = buf_ref{};
}

TEST_CASE("BufferRef Clone #1") {
  buf_table table;
  auto ref1_r = table.request();
  CHECK(ref1_r);
  CHECK(test_index(ref1_r.unwrap()) == 0);
  auto ref2_r = table.request();
  CHECK(ref2_r);
  CHECK(test_index(ref2_r.unwrap()) == 1);

  // Copy value
  auto ref3 = ref2_r.unwrap().copy();
  CHECK(!test_null(ref3));
  CHECK(table.size() == 2);
  CHECK(test_index(ref3) == 1);
}

TEST_CASE("BufferRef Copy #1") {
  buf_table table;
  auto ref1_r = table.request();
  CHECK(ref1_r);
  CHECK(test_index(ref1_r.unwrap()) == 0);
  auto ref2_r = table.request();
  CHECK(ref2_r);
  CHECK(test_index(ref2_r.unwrap()) == 1);

  // Copy
  buf_ref ref3(ref1_r.unwrap());
  CHECK(!test_null(ref3));
  CHECK(table.size() == 2);
  ref3.copy_from(ref2_r.unwrap());
  CHECK(test_index(ref3) == 1);
  CHECK(table.size() == 2);
  CHECK(test_is_used(table, 1) == true);
}

// --- new move tests ---
TEST_CASE("BufferRef Move Operations #1") {
  buf_table table;
  auto ref1_r = table.request();
  CHECK(ref1_r);

  // normal move constructor
  buf_ref ref1 = ref1_r.unwrap();
  buf_ref ref2(std::move(ref1));
  CHECK(test_null(ref1));
  CHECK(!test_null(ref2));
  CHECK(table.size() == 1);

  // move assignment
  buf_ref ref3;
  ref3 = std::move(ref2);
  CHECK(test_null(ref2));
  CHECK(!test_null(ref3));
  CHECK(table.size() == 1);

  // trait_move
  auto ref4 = ref3.move();
  CHECK(test_null(ref3));
  CHECK(!test_null(ref4));
  CHECK(table.size() == 1);

  // move_from via trait
  ref3.move_from(std::move(ref4));
  CHECK(test_null(ref4));
  CHECK(!test_null(ref3));
}

}  // namespace PawnDB
