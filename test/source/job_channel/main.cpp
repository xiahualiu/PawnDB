/**
 * @file main.cpp
 * @brief Channal unit tests
 * @version 0.1
 * @date 2025-01-04
 *
 * @copyright Copyright (c) 2025
 *
 */

#include <cstring>

#include "pawndb/types/buffer_table.h"
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include <sys/socket.h>

#include <chrono>
#include <thread>

#include "doctest/doctest.h"
#include "pawndb/params.h"
#include "pawndb/traits/queue.h"
#include "pawndb/types/job_channel.h"

namespace PawnDB {

TEST_CASE("Channel Empty #1") {
  JobChannel channel;
  CHECK(channel.empty());
  CHECK(!channel.full());

  auto result = channel.get();
  CHECK(!result);
  CHECK(result.getError() == QueueError::Empty);
}

TEST_CASE("Channel Send/Get #1") {
  JobChannel channel;
  CHECK(channel.send({{}, 42, sockaddr_un{}, socklen_t{}}) == QueueError::None);
  CHECK(!channel.empty());
  auto result = channel.get();
  CHECK(result);
  CHECK(result.unwrap().buffer_size() == 42);
  channel.pop();
  CHECK(channel.empty());
}

TEST_CASE("Channel Full #1") {
  JobChannel channel;
  // Fill channel
  for (std::size_t i = 0; i < MAX_ITEM_PER_CHANNEL; i++) {
    CHECK(channel.send({{}, i, sockaddr_un{}, socklen_t{}}) ==
          QueueError::None);
  }
  CHECK(channel.full());
  CHECK(channel.send({{}, 42, sockaddr_un{}, socklen_t{}}) == QueueError::Full);
}

TEST_CASE("Channel Receive Timeout #1") {
  JobChannel channel;
  auto result = channel.recv();
  CHECK(!result);
  CHECK(result.getError() == QueueError::Timeout);
}

TEST_CASE("Channel Multi-threaded #1") {
  JobChannel channel;
  bool received = false;

  std::thread consumer([&]() {
    auto result = channel.recv();
    CHECK(result);
    CHECK(result.unwrap().buffer_size() == 42);
    channel.pop();
    received = true;
  });

  std::thread producer([&]() {
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    CHECK(channel.send({{}, 42, sockaddr_un{}, socklen_t{}}) ==
          QueueError::None);
    channel.notify_not_empty();
  });

  producer.join();
  consumer.join();
  CHECK(received);
}

TEST_CASE("Job Copy #1") {
  Job job1{{}, 42, sockaddr_un{}, socklen_t{110}};
  Job job2 = job1;
  CHECK(job1.buffer_size() == job2.buffer_size());
  CHECK(job1.c_addr_len() == job2.c_addr_len());
}

TEST_CASE("Job Copy #2") {
  Job job1{{}, 42, sockaddr_un{}, socklen_t{110}};
  Job job2;
  job2.copy(job1);
  CHECK(job1.buffer_size() == job2.buffer_size());
  CHECK(job1.c_addr_len() == job2.c_addr_len());
}

TEST_CASE("Job Clone #1") {
  Job job1{{}, 42, sockaddr_un{}, socklen_t{110}};
  Job job2 = job1.clone();
  CHECK(job1.buffer_size() == job2.buffer_size());
  CHECK(job1.c_addr_len() == job2.c_addr_len());
}

TEST_CASE("Job Channel Clear #1") {
  JobChannel channel;
  CHECK(channel.send({{}, 42, sockaddr_un{}, socklen_t{}}) == QueueError::None);
  channel.clear();
  CHECK(channel.empty());
  CHECK(channel.size() == 0);
  CHECK(!channel.full());
}

TEST_CASE("Job Buffer #1") {
  BufferTable table;
  auto buffer_ref = table.request().unwrap();
  Job job1{buffer_ref, 42, sockaddr_un{}, socklen_t{110}};
  CHECK(!buffer_ref._test_null());
  CHECK(job1.buffer()._test_index() == buffer_ref._test_index());
  CHECK(!job1.buffer()._test_null());
  CHECK(job1.buffer_size() == 42);
}

TEST_CASE("Job Address #1") {
  sockaddr_un addr;
  memcpy(&addr, "/tmp/test.sock", sizeof("/tmp/test.sock"));
  Job job1{{}, 42, addr, socklen_t{110}};
  CHECK(job1.c_addr_len() == 110);
  CHECK(memcmp(&addr, job1.c_addr(), sizeof("/tmp/test.sock")) == 0);
}

}  // namespace PawnDB
