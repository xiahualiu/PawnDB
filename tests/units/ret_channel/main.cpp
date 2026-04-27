#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include <sys/socket.h>

#include "doctest/doctest.h"
#include "pawndb/params.h"
#include "pawndb/types/ret_channel.h"

namespace PawnDB {

TEST_CASE("Channel Empty #1") {
  ret_channel channel;
  CHECK(channel.empty());
  CHECK(!channel.full());

  auto result = channel.get();
  CHECK(!result);
  CHECK(result.getError() == QueueError::Empty);
}

TEST_CASE("Channel Send/Get #1") {
  ret_channel channel;
  CHECK(channel.send(42) == QueueError::None);
  CHECK(!channel.empty());
  auto result = channel.get();
  CHECK(result);
  CHECK(result.unwrap() == 42);
  channel.pop();
  CHECK(channel.empty());
}

TEST_CASE("Channel Full #1") {
  ret_channel channel;
  // Fill channel
  for (std::size_t i = 0; i < MAX_TRANSACTIONS; i++) {
    CHECK(channel.send(42) == QueueError::None);
  }
  CHECK(channel.full());
  CHECK(channel.size() == MAX_TRANSACTIONS);
}

TEST_CASE("Channel Clear #1") {
  ret_channel channel;
  CHECK(channel.send(42) == QueueError::None);
  CHECK(!channel.empty());
  channel.clear();
  CHECK(channel.empty());
}

}  // namespace PawnDB
