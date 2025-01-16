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
#include "pawndb/traits/tuple_table.h"
#include "pawndb/types/student_table.h"

namespace PawnDB {

TEST_CASE("Table Basic Operations #1") {
  StudentTable table;
  CHECK(table.empty());
}

TEST_CASE("Table Insert #1") {
  StudentTable table;
  StudentTuple student = {"Brian", 25, 42};
  auto id_r = table.insert(student);
  CHECK(id_r);
  CHECK(!table.empty());
  CHECK(table.size() == 1);
  CHECK(id_r.unwrap().key() == 0);
  CHECK(id_r.unwrap()._test_age() == 25);
  CHECK(id_r.unwrap()._test_name() == "Brian");
  CHECK(!table.full());
}

TEST_CASE("Table Insert #2") {
  StudentTable table;
  StudentTuple student = {"Brian", 25, 42};
  CHECK(table.insert(student));
  CHECK(!table.empty());
  CHECK(!table.full());
  CHECK(table.insert(student));
  CHECK(!table.empty());
  CHECK(!table.full());
  CHECK(table.insert(student));
  CHECK(!table.empty());
  CHECK(!table.full());
  CHECK(table.insert(student));
  CHECK(!table.empty());
  CHECK(!table.full());
  CHECK(table.insert(student));
  CHECK(!table.empty());
  CHECK(!table.full());
  CHECK(table.insert(student));
  CHECK(!table.empty());
  CHECK(!table.full());
  CHECK(table.insert(student));
  CHECK(!table.empty());
  CHECK(!table.full());
  CHECK(table.insert(student));
  CHECK(!table.empty());
  CHECK(!table.full());
  CHECK(table.insert(student));
  CHECK(!table.empty());
  CHECK(!table.full());
  CHECK(table.insert(student));
  CHECK(!table.empty());
  CHECK(table.full());
}

TEST_CASE("Table Lock #1") {
  StudentTable table;
  StudentTuple student = {"Brian", 25, 0};
  auto insert_result = table.insert(student);
  CHECK(insert_result);
  auto wait_r = table.wait_shared();
  CHECK(wait_r);
  CHECK(wait_r.unwrap() == student);
  CHECK(table.size() == 1);
  auto wait_r2 = table._test_get_entry(0);
  CHECK(wait_r2);
  CHECK(wait_r2.unwrap().lock_ == 1);
}

TEST_CASE("Table Lock #2") {
  StudentTable table;
  StudentTuple student = {"Brian", 25, 0};
  auto insert_result = table.insert(student);
  CHECK(insert_result);
  auto wait_r = table.wait_exclusive();
  CHECK(wait_r);
  CHECK(wait_r.unwrap() == student);
  CHECK(table.size() == 1);
  auto wait_r2 = table._test_get_entry(0);
  CHECK(wait_r2);
  CHECK(wait_r2.unwrap().lock_ == -1);
}

TEST_CASE("Table Lock #3") {
  StudentTable table;
  StudentTuple student = {"Brian", 25, 0};
  auto insert_result = table.insert(student);
  CHECK(insert_result);
  auto wait_r = table.wait_shared();
  CHECK(wait_r);
  CHECK(wait_r.unwrap() == student);
  CHECK(table.size() == 1);
  auto wait_r2 = table.wait_exclusive();
  CHECK(wait_r2.getError() == TupleTableError::Timeout);
}

TEST_CASE("Table Lock #4") {
  StudentTable table;
  StudentTuple student = {"Brian", 25, 0};
  auto insert_result = table.insert(student);
  CHECK(insert_result);
  auto wait_r = table.wait_exclusive();
  CHECK(wait_r);
  CHECK(wait_r.unwrap() == student);
  CHECK(table.size() == 1);
  auto wait_r2 = table.wait_shared();
  CHECK(wait_r2.getError() == TupleTableError::Timeout);
}

TEST_CASE("Table Lock #6") {
  StudentTable table;
  StudentTuple student = {"Brian", 25, 0};
  auto insert_result = table.insert(student);
  CHECK(insert_result);
  auto wait_r = table.wait_shared();
  CHECK(wait_r);
  CHECK(wait_r.unwrap() == student);
  CHECK(table.size() == 1);
  auto wait_r2 = table.wait_shared();
  CHECK(wait_r2);
}

TEST_CASE("Table Lock Release #1") {
  StudentTable table;
  StudentTuple student = {"Brian", 25, 0};
  auto insert_result = table.insert(student);
  CHECK(insert_result);
  auto wait_r = table.wait_shared();
  auto wait_r2 = table.wait_shared();
  CHECK(wait_r);
  CHECK(wait_r.unwrap() == student);
  CHECK(wait_r2);
  CHECK(wait_r.unwrap() == student);
  CHECK(table.size() == 1);
  auto get_r = table._test_get_entry(0);
  CHECK(get_r);
  CHECK(get_r.unwrap().lock_ == 2);
  table.release(0);
  CHECK(get_r.unwrap().lock_ == 1);
  table.release(0);
  CHECK(table.size() == 1);
  CHECK(get_r.unwrap().lock_ == 0);
}

TEST_CASE("Table Lock Release #2") {
  StudentTable table;
  StudentTuple student = {"Brian", 25, 0};
  auto insert_result = table.insert(student);
  CHECK(insert_result);
  auto wait_r = table.wait_exclusive();
  auto wait_r2 = table.wait_shared();
  CHECK(wait_r);
  CHECK(wait_r2.getError() == TupleTableError::Timeout);
  CHECK(wait_r.unwrap() == student);
  CHECK(table.size() == 1);
  auto get_r = table._test_get_entry(0);
  CHECK(get_r);
  CHECK(get_r.unwrap().lock_ == -1);
  table.release(0);
  CHECK(get_r.unwrap().lock_ == 0);
  auto wait_r3 = table.wait_shared();
  CHECK(wait_r3);
}

TEST_CASE("Table Timeout #1") {
  StudentTable table;
  auto wait_result = table.wait_shared();
  CHECK(wait_result.getError() == TupleTableError::Timeout);
}

TEST_CASE("Table Timeout #2") {
  StudentTable table;
  auto wait_result = table.wait_exclusive();
  CHECK(wait_result.getError() == TupleTableError::Timeout);
}

TEST_CASE("Table Remove #1") {
  StudentTable table;
  StudentTuple student = {"Brian", 25, 0};
  auto insert_result = table.insert(student);
  CHECK(insert_result);
  table.remove(0);
  CHECK(table.empty());
}

TEST_CASE("Table Notify #1") {
  StudentTable table;
  StudentTuple student = {"Brian", 25, 0};
  auto insert_result = table.insert(student);
  CHECK(insert_result);
  table.notify_not_empty();
  table.notify_not_full();
  table.notify_shared();
  table.notify_exclusive();
}

TEST_CASE("Table Lock Promotion #1") {
  StudentTable table;
  StudentTuple student = {"Brian", 25, 0};
  auto table_r = table.insert(student);
  CHECK(table_r);
  auto wait_r = table.wait_shared();
  CHECK(wait_r);
  auto get_r = table._test_get_entry(0);
  CHECK(get_r);
  CHECK(get_r.unwrap().lock_ == 1);
  CHECK(table.promote(0) == TupleTableError::None);
  CHECK(get_r.unwrap().lock_ == -1);
}


TEST_CASE("Table Lock Promotion #3") {
  StudentTable table;
  StudentTuple student = {"Brian", 25, 0};
  auto table_r = table.insert(student);
  CHECK(table_r);
  auto wait1_r = table.wait_shared();
  auto wait2_r = table.wait_shared();
  CHECK(wait1_r);
  CHECK(wait2_r);
  auto get_r = table._test_get_entry(0);
  CHECK(get_r);
  CHECK(get_r.unwrap().lock_ == 2);
  CHECK(table.promote(0) == TupleTableError::Timeout);
}

}  // namespace PawnDB
