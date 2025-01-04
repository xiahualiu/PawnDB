/**
 * @file main.cpp
 * @brief Queue unit test
 * @version 0.1
 * @date 2025-01-04
 *
 * @copyright Copyright (c) 2025
 *
 */

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include "doctest/doctest.h"
#include "pawndb/ds/queue.h"

namespace PawnDB {

TEST_CASE("Queue Empty #1") {
  Queue<int, 4> queue;
  CHECK(queue.empty());
  CHECK(!queue.full());
}

TEST_CASE("Queue Push #1") {
  Queue<int, 4> queue;
  queue.push(1);
  CHECK(!queue.empty());
  CHECK(!queue.full());
}

TEST_CASE("Queue Full #1") {
  Queue<int, 2> queue;
  queue.push(1);
  queue.push(2);
  CHECK(queue.full());
  CHECK(!queue.empty());
}

TEST_CASE("Queue Full #2") {
  Queue<int, 2> queue;
  int value = 1;
  queue.push(value);
  CHECK(!queue.empty());
}

TEST_CASE("Queue Pop #1") {
  Queue<int, 4> queue;
  queue.push(1);
  queue.push(2);
  CHECK(queue.front() == 1);
  queue.pop();
  CHECK(queue.front() == 2);
  queue.pop();
}

}  // namespace PawnDB
