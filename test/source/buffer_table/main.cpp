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
  auto ref1_r = table.request();
  CHECK(ref1_r);
  auto ref1 = ref1_r.unwrap();
  CHECK(ref1._test_index() == 0);
  CHECK(table.size() == 1);
}

TEST_CASE("BufferRef Thread Safety") {
  BufferTable table = {};
  std::vector<std::thread> threads;
  std::atomic<int> success_count{0};

  for (int i = 0; i < 10; i++) {
    threads.emplace_back([&]() {
      auto ref = table.request();
      if (ref) success_count++;
      ref.unwrap().release();
    });
  }

  for (auto& t : threads) {
    t.join();
  }

  CHECK(success_count > 0);
  CHECK(table.size() == 0);
}

TEST_CASE("BufferRef Index") {
  BufferTable table = {};
  auto ref = table.request().unwrap();
  ref.to_array() = {1, 2, 3, 4, 5};
  auto ref2 = BufferRef();
  ref2 = ref;
  CHECK(std::equal(ref2.to_array().begin(), ref2.to_array().end(),
                   ref.to_array().begin()));
}

}  // namespace PawnDB
