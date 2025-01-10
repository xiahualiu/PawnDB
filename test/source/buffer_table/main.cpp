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

#include <thread>
#include <vector>

#include "doctest/doctest.h"
#include "pawndb/types/buffer_table.h"

namespace PawnDB {

TEST_CASE("BufferRef #1") {
  BufferTable table = {};
  auto ref1 = table.request();
  CHECK(ref1);
  table.release(ref1.unwrap());
}

TEST_CASE("BufferRef Copy #1") {
  BufferTable table = {};
  auto ref1 = BufferFunc::request(table);
  auto ref2(ref1);  // Copy constructor
  auto ref1_index = ref1._test_index();
  auto ref2_index = ref2._test_index();
  CHECK(ref1_index == ref2_index);
  CHECK(table.ref_counts[ref1_index] == 2);
}

TEST_CASE("BufferRef Copy Assignment #1") {
  BufferTable table = {};
  auto ref1 = BufferFunc::request(table);
  auto ref2 = BufferRef();
  ref2 = ref1;  // Copy assignment
  auto ref1_index = ref1._test_index();
  INFO("The value of ref1_index is ", ref1_index);
  auto ref2_index = ref2._test_index();
  INFO("The value of ref2_index is ", ref2_index);
  CHECK(ref1_index == ref2_index);
  CHECK(table.ref_counts[ref1_index] == 2);
}

TEST_CASE("BufferRef Copy Assignment #2") {
  BufferTable table = {};
  auto ref1 = BufferFunc::request(table);
  auto ref1_index = ref1._test_index();
  auto ref2 = BufferFunc::request(table);
  auto ref2_index_before = ref2._test_index();
  ref2 = ref1;  // Copy assignment
  auto ref2_index_after = ref2._test_index();
  CHECK(ref1_index == ref2_index_after);
  CHECK(table.ref_counts[ref1_index] == 2);
  CHECK(table.ref_counts[ref2_index_before] == 0);
}

TEST_CASE("BufferRef Copy Assignment #3") {
  BufferTable table = {};
  auto ref1 = BufferRef();
  auto ref2 = BufferFunc::request(table);
  auto ref2_index_before = ref2._test_index();
  ref2 = ref1;  // Copy assignment to null object
  CHECK(table.ref_counts[ref2_index_before] == 0);
}

TEST_CASE("BufferRef Copy Assignment #4") {
  auto ref1 = BufferRef();
  auto ref2 = BufferRef();
  ref2 = ref1;  // Copy assignment to null object
}

TEST_CASE("BufferRef Move #1") {
  BufferTable table = {};
  auto ref1 = BufferFunc::request(table);
  auto ref1_index = ref1._test_index();
  auto ref2 = std::move(ref1);  // Move constructor
  CHECK(!ref1);                 // Original should be null
  CHECK(ref2);
  CHECK(ref2._test_index() == ref1_index);
  CHECK(table.ref_counts[ref1_index] == 1);
}

TEST_CASE("BufferRef Move Assignment #1") {
  BufferTable table = {};
  auto ref1 = BufferFunc::request(table);
  auto ref1_index = ref1._test_index();
  auto ref2 = BufferFunc::request(table);
  auto ref2_index_before = ref2._test_index();
  ref2 = std::move(ref1);  // Move assignment
  auto ref2_index_after = ref2._test_index();
  CHECK(ref2);
  CHECK(!ref1);  // Original should be null
  CHECK(ref2_index_after == ref1_index);
  CHECK(table.ref_counts[ref2_index_before] == 0);
}

TEST_CASE("BufferRef Move Assignment #2") {
  BufferTable table = {};
  auto ref1 = BufferRef();
  auto ref2 = BufferFunc::request(table);
  auto ref2_index_before = ref2._test_index();
  ref2 = std::move(ref1);  // Copy assignment to null object
  CHECK(table.ref_counts[ref2_index_before] == 0);
}

TEST_CASE("BufferRef Move Assignment #3") {
  auto ref1 = BufferRef();
  auto ref2 = BufferRef();
  ref2 = std::move(ref1);  // Copy assignment to null object
}

TEST_CASE("BufferRef Thread Safety") {
  BufferTable table = {};
  std::vector<std::thread> threads;
  std::atomic<int> success_count{0};

  for (int i = 0; i < 10; i++) {
    threads.emplace_back([&]() {
      auto ref = BufferFunc::request(table);
      if (ref) success_count++;
    });
  }

  for (auto& t : threads) {
    t.join();
  }

  CHECK(success_count > 0);
}

TEST_CASE("BufferRef Index") {
  BufferTable table = {};
  auto ref = BufferFunc::request(table);
  (*ref)[0] = 42;
  CHECK((*ref)[0] == 42);
  auto ref2 = ref;
  CHECK((*ref2)[0] == 42);
  (*ref2)[0] = 43;
  CHECK((*ref)[0] == 43);
}

}  // namespace PawnDB
