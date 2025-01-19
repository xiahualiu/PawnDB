#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include "doctest/doctest.h"
#include "pawndb/types/table_tuple_key.h"

namespace PawnDB {

TEST_CASE("TableTupleKey Constructor #1") {
  TableTupleKey key;
  auto [table_id, tuple_key] = key.disassemble();
  CHECK(table_id == 0);
  CHECK(tuple_key == 0);
}

TEST_CASE("TableTupleKey Constructor #2") {
  TableTupleKey key(1, 2);
  auto [table_id, tuple_key] = key.disassemble();
  CHECK(table_id == 1);
  CHECK(tuple_key == 2);
}

TEST_CASE("TableTupleKey Copy #1") {
  TableTupleKey key1(1, 2);
  TableTupleKey key2 = key1;
  auto [table_id, tuple_key] = key2.disassemble();
  CHECK(table_id == 1);
  CHECK(tuple_key == 2);
}

TEST_CASE("TableTupleKey Assignment #1") {
  TableTupleKey key1(1, 2);
  TableTupleKey key2;
  key2 = key1;
  auto [table_id, tuple_key] = key2.disassemble();
  CHECK(table_id == 1);
  CHECK(tuple_key == 2);
}

TEST_CASE("TableTupleKey Assemble #1") {
  TableTupleKey key;
  key.assemble(1, 2);
  auto [table_id, tuple_key] = key.disassemble();
  CHECK(table_id == 1);
  CHECK(tuple_key == 2);
}

TEST_CASE("TableTupleKey Hash #1") {
  TableTupleKey key(1, 2);
  CHECK(key.hash() == 1);  // Hash uses table_id
}

TEST_CASE("TableTupleKey Equality #1") {
  TableTupleKey key1(1, 2);
  TableTupleKey key2(1, 2);
  TableTupleKey key3(2, 2);
  CHECK(key1 == key2);
  CHECK(key1 != key3);
}

TEST_CASE("TableTupleKey Clone #1") {
  TableTupleKey key1(1, 2);
  auto key2 = key1.clone();
  CHECK(key1 == key2);
}

TEST_CASE("TableTupleKey Copy Trait #1") {
  TableTupleKey key1(1, 2);
  TableTupleKey key2;
  key2.copy(key1);
  CHECK(key1 == key2);
}

}  // namespace PawnDB
