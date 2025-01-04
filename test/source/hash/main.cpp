/**
 * @file main.cpp
 * @brief Hash table unit test
 * @version 0.1
 * @date 2025-01-04
 *
 * @copyright Copyright (c) 2025
 *
 */

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include "doctest/doctest.h"
#include "pawndb/ds/hash.h"

namespace PawnDB {

TEST_CASE("Hash Table Empty #1.") {
  Hash<std::uint8_t, 10> hash_table;
  CHECK(hash_table.empty());
  CHECK(!hash_table.full());
}

TEST_CASE("Hash Test Insert #1")
{
  Hash<std::uint8_t, 10> hash_table;
  CHECK(hash_table.insert(1));
  CHECK(!hash_table.empty());
  CHECK(!hash_table.full());
  auto value = hash_table.search(1);
  CHECK(value);
}

TEST_CASE("Hash Test Insert #2")
{
  Hash<std::uint8_t, 10> hash_table;
  CHECK(hash_table.insert(1));
  CHECK(!hash_table.insert(1));
  CHECK(!hash_table.empty());
  CHECK(!hash_table.full());
  CHECK(hash_table.search(1));
}

TEST_CASE("Hash Test Insert #3")
{
  Hash<std::uint8_t, 10> hash_table;
  for (std::uint8_t i = 0; i < 10; i++) {
    CHECK(hash_table.insert(i));
  }
  CHECK(hash_table.full());
  CHECK(!hash_table.insert(10));
}

TEST_CASE("Hash Test Search #1")
{
  Hash<std::uint8_t, 10> hash_table;
  auto search_r = hash_table.search(1);
  CHECK(search_r.getError()==HashError::NotFound);
}

TEST_CASE("Hash Test Search #2")
{
  Hash<std::uint8_t, 10> hash_table;
  hash_table.insert(1);
  auto search_r = hash_table.search(1);
  CHECK(search_r);
}

TEST_CASE("Hash Test Search #3")
{
  Hash<std::uint8_t, 10> hash_table;
  hash_table.insert(1);
  auto search_r = hash_table.search(2);
  CHECK(search_r.getError()==HashError::NotFound);
}

TEST_CASE("Hash Test Remove #1")
{
  Hash<std::uint8_t, 10> hash_table;
  hash_table.insert(1);
  CHECK(hash_table.remove(1));
  CHECK(hash_table.empty());
}

TEST_CASE("Hash Test Remove #2")
{
  Hash<std::uint8_t, 10> hash_table;
  hash_table.insert(1);
  CHECK(hash_table.remove(1));
  CHECK(!hash_table.remove(1));
  CHECK(hash_table.empty());
}

TEST_CASE("Hash Test Remove #3")
{
  Hash<std::uint8_t, 10> hash_table;
  hash_table.insert(1);
  hash_table.insert(2);
  CHECK(hash_table.remove(1));
  CHECK(hash_table.search(2));
}

TEST_CASE("Hash Test Remove #4")
{
  Hash<std::uint8_t, 10> hash_table;
  hash_table.insert(1);
  hash_table.insert(2);
  CHECK(hash_table.remove(1));
  CHECK(!hash_table.remove(1));
  CHECK(hash_table.search(2));
}

TEST_CASE("Hash Table Remove #5") {
    Hash<std::uint8_t, 4> hash_table;
    
    // Insert and verify
    CHECK(hash_table.insert(1));
    CHECK(!hash_table.empty());
    
    // Remove and verify
    auto remove_result = hash_table.remove(1);
    CHECK(remove_result);
    CHECK(hash_table.empty());
    
    // Try to find removed key
    auto search_result = hash_table.search(1);
    CHECK(!search_result);
    CHECK(search_result.getError() == HashError::NotFound);
}

TEST_CASE("Hash Table Remove #6") {
    Hash<std::uint8_t, 4> hash_table;
    
    CHECK(hash_table.insert(0));  // Will hash to 0
    CHECK(hash_table.insert(4));  // Will hash to 0, probe to 1
    CHECK(hash_table.insert(8));  // Will hash to 0, probe to 2
    CHECK(hash_table.insert(12));  // Will hash to 0, probe to 2
    
    auto result = hash_table.remove(16);
    CHECK(!result);
    CHECK(result.getError() == HashError::NotFound);
}

TEST_CASE("Hash Table Index #1") {
    Hash<std::uint8_t, 4> hash_table;
    
    CHECK(hash_table.insert(4));  // Will hash to 0
    CHECK(hash_table[0] == 4);
}

TEST_CASE("Hash Table Is Valid #1") {
    Hash<std::uint8_t, 4> hash_table;
    
    CHECK(hash_table.insert(4));  // Will hash to 0
    CHECK(hash_table.is_valid(0));
    CHECK(!hash_table.is_valid(1));
}

TEST_CASE("Hash Table Search Through Deleted #1") {
    Hash<std::uint8_t, 4> hash_table;
    
    CHECK(hash_table.insert(0));  // Will hash to 0
    CHECK(hash_table.insert(4));  // Will hash to 0, probe to 1
    CHECK(hash_table.insert(8));  // Will hash to 0, probe to 2
    
    CHECK(hash_table.remove(4));
    
    auto result = hash_table.search(8);
    CHECK(result);
    CHECK(result.unwrap() == 2);
}

TEST_CASE("Hash Table Search Through Deleted #2") {
    Hash<std::uint8_t, 4> hash_table;
    
    CHECK(hash_table.insert(0));  // Will hash to 0
    CHECK(hash_table.insert(4));  // Will hash to 0, probe to 1
    CHECK(hash_table.insert(8));  // Will hash to 0, probe to 2
    CHECK(hash_table.insert(12));  // Will hash to 0, probe to 2
    
    auto result = hash_table.search(16);
    CHECK(!result);
    CHECK(result.getError() == HashError::NotFound);
}

}  // namespace PawnDB

