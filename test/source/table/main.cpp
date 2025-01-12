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
#include "pawndb/types/student_tuple.h"

namespace PawnDB {

TEST_CASE("Table Basic Operations #1") {
  StudentTable table;
  CHECK(table.empty());
}

TEST_CASE("Table Insert #1") {
  StudentTable table;
  StudentTableEntry student = {{"Brian", 25}, 42, 0, false, false};
  auto id_r = table.insert(student);
  CHECK(id_r);
  CHECK(!table.empty());
  CHECK(table.size() == 1);
  CHECK(id_r.unwrap().key == 0);
  CHECK(id_r.unwrap().tuple == student.tuple);
  CHECK(!table.full());
}

TEST_CASE("Table Insert #2") {
  StudentTable table;
  StudentTableEntry student = {{"Brian", 25}, 42, 0, false, false};
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
  StudentTableEntry student = {{"Brian", 25}, 42, 0, false, false};
  auto insert_result = table.insert(student);
  CHECK(insert_result);
  auto wait_r = table.wait_shared();
  CHECK(wait_r);
  CHECK(wait_r.unwrap().tuple == student.tuple);
  CHECK(table.size() == 1);
  CHECK(wait_r.unwrap().lock == 1);
}

TEST_CASE("Table Lock #2") {
  StudentTable table;
  StudentTableEntry student = {{"Brian", 25}, 42, 0, false, false};
  auto insert_result = table.insert(student);
  CHECK(insert_result);
  auto wait_r = table.wait_exclusive();
  CHECK(wait_r);
  CHECK(wait_r.unwrap().tuple == student.tuple);
  CHECK(table.size() == 1);
  CHECK(wait_r.unwrap().lock == -1);
}

TEST_CASE("Table Lock #3") {
  StudentTable table;
  StudentTableEntry student = {{"Brian", 25}, 42, 0, false, false};
  auto insert_result = table.insert(student);
  CHECK(insert_result);
  auto wait_r = table.wait_shared();
  CHECK(wait_r);
  CHECK(wait_r.unwrap().tuple == student.tuple);
  CHECK(table.size() == 1);
  CHECK(wait_r.unwrap().lock == 1);
  auto wait_r2 = table.wait_exclusive();
  CHECK(wait_r2.getError() == TupleTableError::Timeout);
}

TEST_CASE("Table Lock #4") {
  StudentTable table;
  StudentTableEntry student = {{"Brian", 25}, 42, 0, false, false};
  auto insert_result = table.insert(student);
  CHECK(insert_result);
  auto wait_r = table.wait_exclusive();
  CHECK(wait_r);
  CHECK(wait_r.unwrap().tuple == student.tuple);
  CHECK(table.size() == 1);
  CHECK(wait_r.unwrap().lock == -1);
  auto wait_r2 = table.wait_exclusive();
  CHECK(wait_r2.getError() == TupleTableError::Timeout);
}

TEST_CASE("Table Lock #5") {
  StudentTable table;
  StudentTableEntry student = {{"Brian", 25}, 42, 0, false, false};
  auto insert_result = table.insert(student);
  CHECK(insert_result);
  auto wait_r = table.wait_exclusive();
  CHECK(wait_r);
  CHECK(wait_r.unwrap().tuple == student.tuple);
  CHECK(table.size() == 1);
  CHECK(wait_r.unwrap().lock == -1);
  auto wait_r2 = table.wait_shared();
  CHECK(wait_r2.getError() == TupleTableError::Timeout);
}

TEST_CASE("Table Lock #6") {
  StudentTable table;
  StudentTableEntry student = {{"Brian", 25}, 42, 0, false, false};
  auto insert_result = table.insert(student);
  CHECK(insert_result);
  auto wait_r = table.wait_shared();
  CHECK(wait_r);
  CHECK(wait_r.unwrap().tuple == student.tuple);
  CHECK(table.size() == 1);
  CHECK(wait_r.unwrap().lock == 1);
  auto wait_r2 = table.wait_shared();
  CHECK(wait_r2);
}

TEST_CASE("Table Lock Release #1") {
  StudentTable table;
  StudentTableEntry student = {{"Brian", 25}, 42, 0, false, false};
  auto insert_result = table.insert(student);
  CHECK(insert_result);
  auto wait_r = table.wait_shared();
  auto wait_r2 = table.wait_shared();
  CHECK(wait_r);
  CHECK(wait_r.unwrap().tuple == student.tuple);
  CHECK(wait_r2);
  CHECK(wait_r.unwrap().tuple == student.tuple);
  CHECK(table.size() == 1);
  CHECK(wait_r2.unwrap().lock == 2);
  table.release(wait_r2.unwrap().key);
  CHECK(wait_r.unwrap().lock == 1);
  table.release(wait_r2.unwrap().key);
  CHECK(table.size() == 1);
  CHECK(wait_r.unwrap().lock == 0);
}

TEST_CASE("Table Lock Release #2") {
  StudentTable table;
  StudentTableEntry student = {{"Brian", 25}, 42, 0, false, false};
  auto insert_result = table.insert(student);
  CHECK(insert_result);
  auto wait_r = table.wait_exclusive();
  auto wait_r2 = table.wait_shared();
  CHECK(wait_r);
  CHECK(wait_r2.getError() == TupleTableError::Timeout);
  CHECK(wait_r.unwrap().tuple == student.tuple);
  CHECK(table.size() == 1);
  CHECK(wait_r.unwrap().lock == -1);
  table.release(wait_r.unwrap().key);
  CHECK(wait_r.unwrap().lock == 0);
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
  StudentTableEntry student = {{"Brian", 25}, 42, 0, false, false};
  auto insert_result = table.insert(student);
  CHECK(insert_result);
  table.remove(insert_result.unwrap().key);
  CHECK(table.empty());
}

TEST_CASE("Table Notify #1") {
  StudentTable table;
  StudentTableEntry student = {{"Brian", 25}, 42, 0, false, false};
  auto insert_result = table.insert(student);
  CHECK(insert_result);
  table.notify_not_empty();
  table.notify_not_full();
  table.notify_s();
  table.notify_x();
}

TEST_CASE("Table Lock Promotion #1") {
  StudentTable table;
  StudentTableEntry student = {{}, 42, 0, false, false};
  auto table_r = table.insert(student);
  CHECK(table_r);
  auto wait_r = table.wait_shared();
  CHECK(wait_r);
  CHECK(wait_r.unwrap().key == 42);
  CHECK(wait_r.unwrap().lock == 1);
  CHECK(table.promote(wait_r.unwrap().key) == TupleTableError::None);
  CHECK(wait_r.unwrap().lock == -1);
}

TEST_CASE("Table Lock Promotion #2") {
  StudentTable table;
  StudentTableEntry student = {{}, 42, 0, false, false};
  auto table_r = table.insert(student);
  CHECK(table_r);
  auto wait_r = table.wait_exclusive();
  CHECK(wait_r);
  CHECK(wait_r.unwrap().key == 42);
  CHECK(wait_r.unwrap().lock == -1);
  CHECK(table.promote(wait_r.unwrap().key) == TupleTableError::None);
  CHECK(wait_r.unwrap().lock == -1);
}

TEST_CASE("Table Lock Promotion #3") {
  StudentTable table;
  StudentTableEntry student = {{}, 42, 0, false, false};
  auto table_r = table.insert(student);
  CHECK(table_r);
  auto wait1_r = table.wait_shared();
  auto wait2_r = table.wait_shared();
  CHECK(wait1_r);
  CHECK(wait2_r);
  CHECK(wait1_r.unwrap().key == 42);
  CHECK(wait2_r.unwrap().lock == 2);
  CHECK(table.promote(wait1_r.unwrap().key) == TupleTableError::Timeout);
  CHECK(wait1_r.unwrap().lock == 2);
}

}  // namespace PawnDB
