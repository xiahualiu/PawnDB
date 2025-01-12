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

#include <sys/socket.h>

#include <chrono>
#include <thread>

#include "doctest/doctest.h"
#include "pawndb/params.h"
#include "pawndb/types/job_channel.h"

namespace PawnDB {

TEST_CASE("Channel Empty #1") {
  JobChannel channel;
  CHECK(channel.empty());
  CHECK(!channel.full());

  auto result = channel.get();
  CHECK(!result);
  CHECK(result.getError() == FIFOError::Empty);
}

TEST_CASE("Channel Send/Get #1") {
  JobChannel channel;
  CHECK(channel.send({{}, 42, sockaddr{}, socklen_t{}}) == FIFOError::None);
  CHECK(!channel.empty());
  auto result = channel.get();
  CHECK(result);
  CHECK(result.unwrap().buffer_size_ == 42);
  channel.pop();
  CHECK(channel.empty());
}

TEST_CASE("Channel Full #1") {
  JobChannel channel;
  // Fill channel
  for (std::size_t i = 0; i < MAX_ITEM_PER_CHANNEL; i++) {
    CHECK(channel.send({{}, i, sockaddr{}, socklen_t{}}) == FIFOError::None);
  }
  CHECK(channel.full());
  CHECK(channel.send({{}, 42, sockaddr{}, socklen_t{}}) == FIFOError::Full);
}

TEST_CASE("Channel Receive Timeout #1") {
  JobChannel channel;
  auto result = channel.recv();
  CHECK(!result);
  CHECK(result.getError() == FIFOError::Timeout);
}

TEST_CASE("Channel Multi-threaded #1") {
  JobChannel channel;
  bool received = false;

  std::thread consumer([&]() {
    auto result = channel.recv();
    CHECK(result);
    CHECK(result.unwrap().buffer_size_ == 42);
    channel.pop();
    received = true;
  });

  std::thread producer([&]() {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    CHECK(channel.send({{}, 42, sockaddr{}, socklen_t{}}) == FIFOError::None);
    channel.notify_not_empty();
  });

  producer.join();
  consumer.join();
  CHECK(received);
}

}  // namespace PawnDB
