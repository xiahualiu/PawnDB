#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include "doctest/doctest.h"
#include "pawndb/params.h"
#include "pawndb/types/lock_records.h"

namespace PawnDB {

TEST_CASE("LockEntry Clone #1") {
  LockEntry lock_entry({1, 1}, LockType::SHARED);
  auto lock_entry_clone = lock_entry.clone();
  CHECK(lock_entry.key() == lock_entry_clone.key());
  CHECK(lock_entry.lock_type() == lock_entry_clone.lock_type());
}

TEST_CASE("LockEntry Copy #1") {
  LockEntry lock_entry({1, 1}, LockType::SHARED);
  LockEntry lock_entry_copy(lock_entry);
  CHECK(lock_entry.key() == lock_entry_copy.key());
  CHECK(lock_entry.lock_type() == lock_entry_copy.lock_type());
}

TEST_CASE("LockEntry Copy #2") {
  LockEntry lock_entry({1, 1}, LockType::SHARED);
  LockEntry lock_entry_copy;
  lock_entry_copy.copy(lock_entry);
  CHECK(lock_entry.key() == lock_entry_copy.key());
  CHECK(lock_entry.lock_type() == lock_entry_copy.lock_type());
}

TEST_CASE("LockTable Search Not Found #1") {
  LockTable lock_table;
  CHECK(lock_table.get_lock({1, 1}).getError() == LockError::NotFound);
}

TEST_CASE("LockTable Search Not Found #2") {
  LockTable lock_table;
  // Insert to full
  for (tbl_row_t i = 0; i < MAX_LOCK_PER_TRANSACTION; i++) {
    CHECK(lock_table.add_lock({1, i}, LockType::SHARED) == LockError::None);
  }
  CHECK(lock_table.get_lock({1, 100}).getError() == LockError::NotFound);
}

TEST_CASE("LockTable Get Lock Not Found #1") {
  LockTable lock_table;
  CHECK(lock_table.get_lock({1, 1}).getError() == LockError::NotFound);
}

TEST_CASE("LockTableIterator Clone #1") {
  LockTable lock_table;
  auto it = lock_table.begin();
  auto it_clone = it.clone();
  CHECK(it._test_index() == it_clone._test_index());
}

TEST_CASE("LockTableIterator Copy #1") {
  LockTable lock_table;
  auto it = lock_table.begin();
  LockTableIterator it_copy(it);
  CHECK(it._test_index() == it_copy._test_index());
}

TEST_CASE("LockTableIterator Copy #2") {
  LockTable lock_table;
  auto it = lock_table.begin();
  auto it2 = lock_table.end();
  it2.copy(it);
  CHECK(it2._test_index() == it._test_index());
}

TEST_CASE("LockTableIterator Copy Assignment #1") {
  LockTable lock_table;
  auto it = lock_table.begin();
  auto it2 = lock_table.end();
  it2 = it;
  CHECK(it2._test_index() == it._test_index());
}

TEST_CASE("LockTable Empty #1") {
  LockTable lock_table;
  CHECK(lock_table.empty());
  CHECK(!lock_table.full());
}

TEST_CASE("LockTable Clear #1") {
  LockTable lock_table;
  CHECK(lock_table.empty());
  CHECK(!lock_table.full());
  CHECK(lock_table.size() == 0);
  lock_table.add_lock({1, 1}, LockType::SHARED);
  lock_table.clear();
  CHECK(lock_table.empty());
  CHECK(!lock_table.full());
  CHECK(lock_table.size() == 0);
}

TEST_CASE("LockTable Basic Lock #1") {
  LockTable lock_table;
  CHECK(lock_table.add_lock({1, 1}, LockType::SHARED) == LockError::None);
  CHECK(!lock_table.empty());
  CHECK(!lock_table.full());
  CHECK(lock_table.size() == 1);
}

TEST_CASE("LockTable Lock Conflict #1") {
  LockTable lock_table;
  CHECK(lock_table.add_lock({1, 1}, LockType::EXCLUSIVE) == LockError::None);
  CHECK(lock_table.add_lock({1, 1}, LockType::SHARED) == LockError::Conflict);
  CHECK(!lock_table.full());
  CHECK(lock_table.size() == 1);
}

TEST_CASE("LockTable Multiple Locks #1") {
  LockTable lock_table;
  CHECK(lock_table.add_lock({1, 1}, LockType::SHARED) == LockError::None);
  CHECK(lock_table.add_lock({1, 2}, LockType::SHARED) == LockError::None);
  CHECK(lock_table.add_lock({2, 1}, LockType::SHARED) == LockError::None);
  CHECK(lock_table.size() == 3);
  CHECK(!lock_table.full());
  CHECK(!lock_table.empty());
}

TEST_CASE("LockTable Lock Promotion #1") {
  LockTable lock_table;
  CHECK(lock_table.add_lock({1, 1}, LockType::SHARED) == LockError::None);
  CHECK(lock_table.promote_lock({1, 1}) == LockError::None);
  auto lock_type = lock_table.get_lock({1, 1});
  CHECK(lock_type);
  CHECK(lock_type.unwrap() == LockType::EXCLUSIVE);
}

TEST_CASE("LockTable Lock Release #1") {
  LockTable lock_table;
  CHECK(lock_table.add_lock({1, 1}, LockType::EXCLUSIVE) == LockError::None);
  CHECK(lock_table.rm_lock({1, 1}) == LockError::None);
  CHECK(lock_table.empty());
}

TEST_CASE("LockTable Resource Exhaustion #1") {
  LockTable lock_table;
  for (tbl_row_t i = 0; i < MAX_LOCK_PER_TRANSACTION; i++) {
    CHECK(lock_table.add_lock({1, i}, LockType::SHARED) == LockError::None);
  }
  CHECK(lock_table.add_lock({1, MAX_LOCK_PER_TRANSACTION}, LockType::SHARED) ==
        LockError::Full);
}

TEST_CASE("LockTable Hash Distribution #1") {
  LockTable lock_table;
  CHECK(lock_table.add_lock({1, 0}, LockType::SHARED) == LockError::None);

  // Get actual storage index
  auto it = lock_table.begin();
  INFO("Storage index:", it._test_index());

  // Verify key at that index
  auto lock_entry = *it;
  auto [tbl, key] = lock_entry.key().disassemble();
  CHECK(tbl == 1);
  CHECK(key == 0);
  CHECK(lock_entry.lock_type() == LockType::SHARED);
}

TEST_CASE("LockTable Iterator #1") {
  LockTable lock_table;
  CHECK(lock_table.add_lock({1, 0}, LockType::SHARED) == LockError::None);
  auto it = lock_table.begin();
  INFO("it: ", it._test_index());
  CHECK(it._test_index() == 1);
  it++;
  CHECK(it._test_index() == MAX_LOCK_PER_TRANSACTION);
  CHECK(it == lock_table.end());
  CHECK(lock_table.rm_lock({1, 0}) == LockError::None);
  auto it2 = lock_table.begin();
  CHECK(it2 == lock_table.end());
}

TEST_CASE("LockTable Iterator #2") {
  LockTable lock_table;
  for (tbl_row_t i = 0; i < MAX_LOCK_PER_TRANSACTION; i++) {
    CHECK(lock_table.add_lock({1, i}, LockType::SHARED) == LockError::None);
  }
  for (auto it = lock_table.begin(); it != lock_table.end(); it++) {
    auto lock_entry = *it;
    auto [tbl, key] = lock_entry.key().disassemble();
    CHECK(tbl == 1);
    CHECK(lock_entry.lock_type() == LockType::SHARED);
    CHECK(lock_table.rm_lock({tbl, key}) == LockError::None);
  }
  CHECK(lock_table.empty());
}

}  // namespace PawnDB
