/**
 * @file main.cpp
 * @brief Tuple serdes unit test
 * @version 0.1
 * @date 2025-01-04
 *
 * @copyright Copyright (c) 2025
 *
 */

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include "doctest/doctest.h"
#include "pawndb/tuple_serdes.h"

namespace PawnDB {

TEST_CASE("TupleSerdes Single Int #1") {
  using TestTable = Table<1, int>;
  TestTable table;
  auto& int_value = std::get<0>(table._test_get_tuple(0));
  int_value = 42;

  std::array<char, BUFFER_WIDTH> buffer{};
  TpSerDes<TestTable> serdes;

  buf_size_t offset = 0;

  // Test serialization
  CHECK(serdes.serialize(table._test_get_tuple(0), buffer, offset));

  // Test deserialization
  std::tuple<int> tuple_compare = {};
  CHECK(serdes.deserialize(buffer, 0, tuple_compare));
  CHECK(std::get<0>(tuple_compare) == 42);
}

TEST_CASE("TupleSerdes Single Int #2") {
  using TestTable1 = Table<1, int>;
  TestTable1 table;
  auto& int_value = std::get<0>(table._test_get_tuple(0));
  int_value = 42;

  std::array<char, BUFFER_WIDTH> buffer{};
  TpSerDes<TestTable1> serdes1;

  buf_size_t offset = 0;

  // Test serialization
  CHECK(serdes1.serialize(table._test_get_tuple(0), buffer, offset));

  using TestTable2 = Table<1, unsigned int>;
  TestTable2 table2;

  TpSerDes<TestTable2> serdes2;

  // Test deserialization
  std::tuple<unsigned int> tuple_compare = {};
  CHECK(serdes2.deserialize(buffer, 0, tuple_compare).getError() ==
        DeserialError::IDMismatch);
}

}  // namespace PawnDB
