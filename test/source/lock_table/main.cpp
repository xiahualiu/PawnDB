/**
 * @file main.cpp
 * @brief Buffer table unit test
 * @version 0.1
 * @date 2025-01-04
 *
 * @copyright Copyright (c) 2025
 *
 */

#include "pawndb/params.h"
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include "doctest/doctest.h"
#include "pawndb/lock_table.h"

namespace PawnDB {

TEST_CASE("LockTable Empty #1") {
  LockTable lock_table;
  CHECK(lock_table.empty());
}

TEST_CASE("LockTable Basic Lock #1") {
  LockTable lock_table;
  CHECK(lock_table.lock(1, 1, LockType::Shared) == LockError::None);
  CHECK(!lock_table.empty());
}

TEST_CASE("LockTable Lock Conflict #1") {
  LockTable lock_table;
  CHECK(lock_table.lock(1, 1, LockType::Exclusive) == LockError::None);
  CHECK(lock_table.lock(1, 1, LockType::Shared) == LockError::LockConflict);
}

TEST_CASE("LockTable Multiple Locks #1") {
  LockTable lock_table;
  CHECK(lock_table.lock(1, 1, LockType::Shared) == LockError::None);
  CHECK(lock_table.lock(1, 2, LockType::Shared) == LockError::None);
  CHECK(lock_table.lock(2, 1, LockType::Shared) == LockError::None);
}

TEST_CASE("LockTable Lock Promotion #1") {
  LockTable lock_table;
  CHECK(lock_table.lock(1, 1, LockType::Shared) == LockError::None);
  CHECK(lock_table.promote(1, 1) == LockError::None);
  auto lock_type = lock_table.get(1, 1);
  CHECK(lock_type);
  CHECK(lock_type.unwrap() == LockType::Exclusive);
}

TEST_CASE("LockTable Lock Release #1") {
  LockTable lock_table;
  CHECK(lock_table.lock(1, 1, LockType::Exclusive) == LockError::None);
  CHECK(lock_table.unlock(1, 1) == LockError::None);
  CHECK(lock_table.empty());
}

TEST_CASE("LockTable Lock Release #2") {
  LockTable lock_table;
  CHECK(lock_table.lock(1, 1, LockType::Exclusive) == LockError::None);
  CHECK(lock_table.unlock(1, 16) == LockError::LockNotFound);
  CHECK(!lock_table.empty());
}

TEST_CASE("LockTable Resource Exhaustion #1") {
  LockTable lock_table;
  for (tbl_row_t i = 0; i < MAX_LOCK_PER_TRANSACTION; i++) {
    CHECK(lock_table.lock(1, i, LockType::Shared) == LockError::None);
  }
  CHECK(lock_table.lock(1, MAX_LOCK_PER_TRANSACTION, LockType::Shared) ==
        LockError::LockFull);
}

TEST_CASE("LockTable Hash Distribution #1") {
  LockTable lock_table;
  CHECK(lock_table.lock(1, 0, LockType::Shared) == LockError::None);

  // Get actual storage index
  auto it = lock_table.begin();
  INFO("Storage index:", it._test_index());

  // Verify key at that index
  auto [tbl, key, lk] = *it;
  CHECK(it._test_index() == 1);
  CHECK(tbl == 1);
  CHECK(key == 0);
  CHECK(lk == LockType::Shared);
}

TEST_CASE("LockTable Iterator #1") {
  LockTable lock_table;
  CHECK(lock_table.lock(1, 0, LockType::Shared) == LockError::None);
  auto it = lock_table.begin();
  INFO("it: ", it._test_index());
  CHECK(it._test_index() == 1);
  it++;
  CHECK(it._test_index() == MAX_LOCK_PER_TRANSACTION);
  CHECK(it == lock_table.end());
  CHECK(lock_table.unlock(1, 0) == LockError::None);
  auto it2 = lock_table.begin();
  CHECK(it2 == lock_table.end());
}

TEST_CASE("LockTable Iterator #2") {
  LockTable lock_table;
  for (tbl_row_t i = 0; i < MAX_LOCK_PER_TRANSACTION; i++) {
    CHECK(lock_table.lock(1, i, LockType::Shared) == LockError::None);
  }
  for (auto it = lock_table.begin(); it != lock_table.end(); it++) {
    auto [tbl, key, lk] = *it;
    CHECK(tbl == 1);
    CHECK(lk == LockType::Shared);
    CHECK(lock_table.unlock(tbl, key) == LockError::None);
  }
  CHECK(lock_table.empty());
}

TEST_CASE("LockTable Key #1") {
  LockTable lock_table;
  auto hash_key = lock_table.key(1, 1);
  auto [tbl, key] = lock_table.key(hash_key);
  CHECK(tbl == 1);
  CHECK(key == 1);
}

}  // namespace PawnDB
