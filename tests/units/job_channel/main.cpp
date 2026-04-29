#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include <chrono>
#include <cstring>
#include <thread>

#include "doctest/doctest.h"
#include "pawndb/params.h"
#include "pawndb/traits/queue.h"
#include "pawndb/types/buf_table.h"
#include "pawndb/types/client_conn.h"
#include "pawndb/types/job_channel.h"

namespace PawnDB {

TEST_CASE("Channel Empty #1") {
  job_channel channel;
  CHECK(channel.empty());
  CHECK(!channel.full());

  auto result = channel.get();
  CHECK(!result);
  CHECK(result.getError() == QueueError::Empty);
}

TEST_CASE("Channel Send/Get #1") {
  job_channel channel;
  CHECK(channel.send({{}, 42, client_conn{}}) == QueueError::None);
  CHECK(!channel.empty());
  auto result = channel.get();
  CHECK(result);
  CHECK(result.unwrap().buffer_size() == 42);
  channel.pop();
  CHECK(channel.empty());
}

TEST_CASE("Channel Full #1") {
  job_channel channel;
  // Fill channel
  for (std::size_t i = 0; i < MAX_ITEM_PER_CHANNEL; i++) {
    CHECK(channel.send({{}, i, client_conn{}}) == QueueError::None);
  }
  CHECK(channel.full());
  CHECK(channel.send({{}, 42, client_conn{}}) == QueueError::Full);
}

TEST_CASE("Channel Receive Timeout #1") {
  job_channel channel;
  auto result = channel.recv();
  CHECK(!result);
  CHECK(result.getError() == QueueError::Timeout);
}

TEST_CASE("Channel Multi-threaded #1") {
  job_channel channel;
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
    CHECK(channel.send({{}, 42, client_conn{}}) == QueueError::None);
    channel.notify_not_empty();
  });

  producer.join();
  consumer.join();
  CHECK(received);
}

TEST_CASE("Job Copy #1") {
  job job1{{}, 42, client_conn{}};
  job job2 = job1;
  CHECK(job1.buffer_size() == job2.buffer_size());
}

TEST_CASE("Job Copy #2") {
  job job1{{}, 42, client_conn{}};
  job job2;
  job2.copy_from(job1);
  CHECK(job1.buffer_size() == job2.buffer_size());
}

TEST_CASE("Job CopyValue #1") {
  job job1{{}, 42, client_conn{}};
  job job2 = job1.copy();
  CHECK(job1.buffer_size() == job2.buffer_size());
}

TEST_CASE("Job Channel Clear #1") {
  job_channel channel;
  CHECK(channel.send({{}, 42, client_conn{}}) == QueueError::None);
  channel.clear();
  CHECK(channel.empty());
  CHECK(channel.size() == 0);
  CHECK(!channel.full());
}

TEST_CASE("Job Buffer #1") {
  buf_table table;
  auto buffer_ref = table.request().unwrap();
  job job1{buffer_ref, 42, client_conn{}};
  CHECK(!buffer_ref._test_null());
  CHECK(job1.buffer()._test_index() == buffer_ref._test_index());
  CHECK(!job1.buffer()._test_null());
  CHECK(job1.buffer_size() == 42);
}

TEST_CASE("Job Address #1") {
  sockaddr_un addr;
  std::memcpy(&addr, "/tmp/test.sock", sizeof("/tmp/test.sock"));
  auto conn = client_conn(addr, socklen_t{110});
  job job1{{}, 42, conn};
  CHECK(job1.buffer_size() == 42);
}

}  // namespace PawnDB
