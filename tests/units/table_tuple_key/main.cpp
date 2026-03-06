#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include <utility>

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

TEST_CASE("TableTupleKey CopyValue #1") {
  TableTupleKey key1(1, 2);
  auto key2 = key1.copy();
  CHECK(key1 == key2);
}

TEST_CASE("TableTupleKey Copy Trait #1") {
  TableTupleKey key1(1, 2);
  TableTupleKey key2;
  key2.copy_from(key1);
  CHECK(key1 == key2);
}

TEST_CASE("TableTupleKey Move Trait #1") {
  TableTupleKey key1(3, 4);

  // move constructor
  TableTupleKey key2(std::move(key1));
  auto [t2, r2] = key2.disassemble();
  CHECK(t2 == 3);
  CHECK(r2 == 4);

  // moved-from state: behavior is unspecified but comparison should still work
  // we don't rely on a particular value, just that key2 holds correct data.

  // move assignment
  TableTupleKey key3;
  key3 = std::move(key2);
  auto [t3, r3] = key3.disassemble();
  CHECK(t3 == 3);
  CHECK(r3 == 4);

  // trait_move and move_from
  auto key4 = key3.move();
  auto [t4, r4] = key4.disassemble();
  CHECK(t4 == 3);
  CHECK(r4 == 4);

  key3.move_from(std::move(key4));
  auto [t3a, r3a] = key3.disassemble();
  CHECK(t3a == 3);
  CHECK(r3a == 4);
}

}  // namespace PawnDB
