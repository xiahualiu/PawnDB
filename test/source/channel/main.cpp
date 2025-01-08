/**
 * @file main.cpp
 * @brief Channal unit tests
 * @version 0.1
 * @date 2025-01-04
 *
 * @copyright Copyright (c) 2025
 *
 */

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include <chrono>
#include <thread>

#include "doctest/doctest.h"
#include "pawndb/channel.h"

namespace PawnDB {

TEST_CASE("Channel Empty #1") {
  Channel<int> channel;
  CHECK(channel.empty());
  CHECK(!channel.full());

  auto result = channel.get();
  CHECK(!result);
  CHECK(result.getError() == FIFOError::GetNothing);
  channel.pop();
}

TEST_CASE("Channel Send/Get #1") {
  Channel<int> channel;
  CHECK(channel.send(42) == FIFOError::None);

  auto result = channel.get();
  CHECK(result);
  CHECK(result.unwrap() == 42);
  channel.pop();
}

TEST_CASE("Channel Full #1") {
  Channel<int> channel;

  // Fill channel
  for (int i = 0; i < CHANNEL_ROWS; i++) {
    CHECK(channel.send(i) == FIFOError::None);
  }

  CHECK(channel.full());
  CHECK(channel.send(42) == FIFOError::TableFull);
}

TEST_CASE("Channel Full #2") {
  Channel<int> channel;

  // Fill channel
  for (int i = 0; i < CHANNEL_ROWS; i++) {
    CHECK(channel.send(i) == FIFOError::None);
  }

  CHECK(channel.full());
  int value = 42;
  CHECK(channel.send(value) == FIFOError::TableFull);
}

TEST_CASE("Channel Receive Timeout #1") {
  Channel<int> channel;
  auto result = channel.recv();
  CHECK(!result);
  channel.pop();
  CHECK(result.getError() == FIFOError::Timeout);
}

TEST_CASE("Channel Multi-threaded #1") {
  Channel<int> channel;
  bool received = false;

  std::thread consumer([&]() {
    auto result = channel.recv();
    CHECK(result);
    CHECK(result.unwrap() == 42);
    channel.pop();
    received = true;
  });

  std::thread producer([&]() {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    CHECK(channel.send(42) == FIFOError::None);
    channel.notify();
  });

  producer.join();
  consumer.join();
  CHECK(received);
}

}  // namespace PawnDB
