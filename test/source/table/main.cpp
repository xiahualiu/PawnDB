/**
 * @file main.cpp
 * @brief Table unit test
 * @version 0.1
 * @date 2025-01-04
 *
 * @copyright Copyright (c) 2025
 *
 */

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include "doctest/doctest.h"
#include "pawndb/table.h"

namespace PawnDB {

TEST_CASE("Table Basic Operations #1") {
  Table<10, int, int> table;
  CHECK(table._test_empty());
}

TEST_CASE("Table Insert #1") {
  Table<10, int, bool> table;
  auto tuple = std::make_tuple(42, false);
  auto id_r = table.insert(tuple);
  CHECK(id_r);
  CHECK(!table._test_empty());
  CHECK(table._test_size() == 1);
  CHECK(table._test_get_tuple(id_r.unwrap()) == tuple);
  CHECK(table.key_at(id_r.unwrap()) == 0);
  CHECK(!table._test_full());
}

TEST_CASE("Table Insert #2") {
  Table<10, int, bool> table;
  auto tuple = std::make_tuple(42, false);
  CHECK(table.insert(tuple));
  CHECK(!table._test_empty());
  CHECK(!table._test_full());
  CHECK(table.insert(tuple));
  CHECK(!table._test_empty());
  CHECK(!table._test_full());
  CHECK(table.insert(tuple));
  CHECK(!table._test_empty());
  CHECK(!table._test_full());
  CHECK(table.insert(tuple));
  CHECK(!table._test_empty());
  CHECK(!table._test_full());
  CHECK(table.insert(tuple));
  CHECK(!table._test_empty());
  CHECK(!table._test_full());
  CHECK(table.insert(tuple));
  CHECK(!table._test_empty());
  CHECK(!table._test_full());
  CHECK(table.insert(tuple));
  CHECK(!table._test_empty());
  CHECK(!table._test_full());
  CHECK(table.insert(tuple));
  CHECK(!table._test_empty());
  CHECK(!table._test_full());
  CHECK(table.insert(tuple));
  CHECK(!table._test_empty());
  CHECK(!table._test_full());
  CHECK(table.insert(tuple));
  CHECK(!table._test_empty());
  CHECK(table._test_full());
}

TEST_CASE("Table Lock #1") {
  Table<10, int, bool> table;
  auto tuple = std::make_tuple(42, false);
  auto insert_result = table.insert(tuple);
  CHECK(insert_result);

  auto wait_r = table.wait_s();
  CHECK(wait_r);
  CHECK(table._test_get_tuple(wait_r.unwrap()) == tuple);
  CHECK(table._test_size() == 1);
  CHECK(table._test_get_lock(wait_r.unwrap()) == 1);
}

TEST_CASE("Table Lock #2") {
  Table<10, int, bool> table;
  auto tuple = std::make_tuple(42, false);
  auto insert_result = table.insert(tuple);
  CHECK(insert_result);

  auto wait_r = table.wait_x();
  CHECK(wait_r);
  CHECK(table._test_get_tuple(wait_r.unwrap()) == tuple);
  CHECK(table._test_size() == 1);
  CHECK(table._test_get_lock(wait_r.unwrap()) == -1);
}

TEST_CASE("Table Lock #3") {
  Table<10, int, bool> table;
  auto tuple = std::make_tuple(42, false);
  auto insert_result = table.insert(tuple);
  CHECK(insert_result);

  auto wait_r = table.wait_s();
  CHECK(wait_r);
  CHECK(table._test_get_tuple(wait_r.unwrap()) == tuple);
  CHECK(table._test_size() == 1);
  CHECK(table._test_get_lock(wait_r.unwrap()) == 1);
  auto wait_r2 = table.wait_x();
  CHECK(wait_r2.getError() == TableError::Timeout);
}

TEST_CASE("Table Lock #4") {
  Table<10, int, bool> table;
  auto tuple = std::make_tuple(42, false);
  auto insert_result = table.insert(tuple);
  CHECK(insert_result);

  auto wait_r = table.wait_x();
  CHECK(wait_r);
  CHECK(table._test_get_tuple(wait_r.unwrap()) == tuple);
  CHECK(table._test_size() == 1);
  CHECK(table._test_get_lock(wait_r.unwrap()) == -1);
  auto wait_r2 = table.wait_s();
  CHECK(wait_r2.getError() == TableError::Timeout);
}

TEST_CASE("Table Lock Release #1") {
  Table<10, int, bool> table;
  auto tuple = std::make_tuple(42, false);
  auto insert_result = table.insert(tuple);
  CHECK(insert_result);

  auto wait_r = table.wait_s();
  auto wait_r2 = table.wait_s();
  CHECK(wait_r);
  CHECK(wait_r2);
  CHECK(table._test_get_tuple(wait_r.unwrap()) == tuple);
  CHECK(table._test_size() == 1);
  CHECK(table._test_get_lock(wait_r.unwrap()) == 2);
  auto key = table.key_at(wait_r.unwrap());
  table.release_s(key);
  CHECK(table._test_get_lock(wait_r.unwrap()) == 1);
  table.release_s(key);
  CHECK(table._test_get_lock(wait_r.unwrap()) == 0);
}

TEST_CASE("Table Lock Release #2") {
  Table<10, int, bool> table;
  auto tuple = std::make_tuple(42, false);
  auto insert_result = table.insert(tuple);
  CHECK(insert_result);

  auto wait_r = table.wait_x();
  auto wait_r2 = table.wait_s();
  CHECK(wait_r);
  CHECK(wait_r2.getError() == TableError::Timeout);
  CHECK(table._test_get_tuple(wait_r.unwrap()) == tuple);
  CHECK(table._test_size() == 1);
  CHECK(table._test_get_lock(wait_r.unwrap()) == -1);
  auto key = table.key_at(wait_r.unwrap());
  table.release_x(key);
  CHECK(table._test_get_lock(wait_r.unwrap()) == 0);
  auto wait_r3 = table.wait_s();
  CHECK(wait_r3);
}

TEST_CASE("Table Timeout #1") {
  Table<10, int, std::string> table;
  auto wait_result = table.wait_s();
  CHECK(wait_result.getError() == TableError::Timeout);
}

TEST_CASE("Table Timeout #2") {
  Table<10, int, std::string> table;
  auto wait_result = table.wait_x();
  CHECK(wait_result.getError() == TableError::Timeout);
}

TEST_CASE("Table Remove #1") {
  Table<10, int, bool> table;
  auto tuple = std::make_tuple(42, false);
  auto insert_result = table.insert(tuple);
  CHECK(insert_result);
  auto key = table.key_at(insert_result.unwrap());
  table.remove(key);
  CHECK(table._test_empty());
}

TEST_CASE("Table Update #1") {
  Table<10, int, bool> table;
  auto tuple = std::make_tuple(42, false);
  auto insert_result = table.insert(tuple);
  CHECK(insert_result);
  auto key = table.key_at(insert_result.unwrap());
  auto new_tuple = std::make_tuple(42, true);
  table.update(key, new_tuple);
  CHECK(table._test_get_tuple(key) == new_tuple);
}

TEST_CASE("Table Notify #1") {
  Table<10, int, bool> table;
  auto tuple = std::make_tuple(42, false);
  auto insert_result = table.insert(tuple);
  CHECK(insert_result);
  table.notify_not_empty();
  table.notify_not_full();
  table.notify_s_available();
  table.notify_x_available();
}

TEST_CASE("Table Lock Promotion #1") {
  Table<10, int, bool> table;
  auto tuple = std::make_tuple(42, true);
  auto idx = table.insert(tuple);
  CHECK(idx);
  auto wait_r = table.wait_s();
  CHECK(wait_r);
  CHECK(table._test_get_tuple(wait_r.unwrap()) == tuple);
  CHECK(table._test_get_lock(wait_r.unwrap()) == 1);
  auto key = table.key_at(wait_r.unwrap());
  CHECK(table.promote(key) == TableError::None);
  CHECK(table._test_get_lock(wait_r.unwrap()) == -1);
}

TEST_CASE("Table Lock Promotion #2") {
  Table<10, int, bool> table;
  auto tuple = std::make_tuple(42, true);
  auto idx = table.insert(tuple);
  CHECK(idx);
  auto wait_r = table.wait_x();
  CHECK(wait_r);
  CHECK(table._test_get_tuple(wait_r.unwrap()) == tuple);
  CHECK(table._test_get_lock(wait_r.unwrap()) == -1);
  auto key = table.key_at(wait_r.unwrap());
  CHECK(table.promote(key) == TableError::None);
  CHECK(table._test_get_lock(wait_r.unwrap()) == -1);
}

TEST_CASE("Table Lock Promotion #3") {
  Table<10, int, bool> table;
  auto tuple = std::make_tuple(42, true);
  auto idx = table.insert(tuple);
  CHECK(idx);
  auto wait_r = table.wait_s();
  auto wait_r2 = table.wait_s();
  CHECK(wait_r);
  CHECK(wait_r2);
  CHECK(wait_r.unwrap() == wait_r2.unwrap());
  CHECK(table._test_get_tuple(wait_r.unwrap()) == tuple);
  CHECK(table._test_get_lock(wait_r.unwrap()) == 2);
  auto key = table.key_at(wait_r.unwrap());
  CHECK(table.promote(key) == TableError::Timeout);
  CHECK(table._test_get_lock(wait_r.unwrap()) == 2);
}

TEST_CASE("Table Index #1") {
  Table<10, int, bool> table;
  auto tuple = std::make_tuple(42, true);
  auto idx = table.insert(tuple);
  CHECK(idx);
  CHECK(table[idx.unwrap()] == tuple);
}

}  // namespace PawnDB
