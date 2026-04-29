#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include "doctest/doctest.h"
#include "pawndb/params.h"
#include "pawndb/traits/tuple_table.h"
#include "pawndb/types/buf_table.h"
#include "pawndb/types/student_table.h"

namespace PawnDB {

TEST_CASE("StudentTuple Copy #1") {
  StudentTuple student = {"Brian", 25, 42};
  StudentTuple student2 = student;
  CHECK(student2._test_name() == "Brian");
  CHECK(student2._test_age() == 25);
  CHECK(student2.key() == 42);
}

TEST_CASE("StudentTuple Copy #2") {
  StudentTuple student = {"Brian", 25, 42};
  StudentTuple student2 = student;
  CHECK(student2._test_name() == "Brian");
  CHECK(student2._test_age() == 25);
  CHECK(student2.key() == 42);
}

TEST_CASE("StudentTuple Copy Trait #1") {
  StudentTuple student = {"Brian", 25, 42};
  StudentTuple student2;
  student2.copy_from(student);
  CHECK(student2._test_name() == "Brian");
  CHECK(student2._test_age() == 25);
  CHECK(student2.key() == 42);
}

TEST_CASE("StudentTuple CopyValue Trait #1") {
  StudentTuple student = {"Brian", 25, 42};
  auto student2 = student.copy();
  CHECK(student2._test_name() == "Brian");
  CHECK(student2._test_age() == 25);
  CHECK(student2.key() == 42);
}

TEST_CASE("StudentTuple Checksum #1") {
  StudentTuple student = {"Brian", 25, 42};
  student.set_checksum();
  CHECK(student.val_checksum());
}

TEST_CASE("StudentTuple Tickstamp #1") {
  StudentTuple student = {"Brian", 25, 42};
  student.set_tickstamp(100);
  CHECK(student.read_tickstamp() == 100);
}

TEST_CASE("StudentTuple Hash #1") {
  StudentTuple student = {"Brian", 25, 42};
  CHECK(student.hash() == 42);
}

TEST_CASE("StudentTuple Serialize/Deserialize #1") {
  StudentTuple student = {"Brian", 25, 42};
  student.set_checksum();
  buf_table buffer_table;
  auto buffer = buffer_table.request().unwrap();
  auto r = student.serialize(buffer, 0);
  CHECK(r.unwrap() == 37);
  auto new_tuple = StudentTuple();
  auto r2 = new_tuple.deserialize(buffer, 0);
  CHECK(r2.unwrap() == 37);
  CHECK(new_tuple._test_name() == "Brian");
  CHECK(new_tuple._test_age() == 25);
  CHECK(new_tuple.val_checksum());
}

TEST_CASE("StudentTable Search #1") {
  StudentTable table;
  StudentTuple student = {"Brian", 25, 42};
  auto id_r = table.insert(student);
  CHECK(id_r);
  auto search_r = table.search(0);
  CHECK(search_r);
  CHECK(search_r.unwrap()._test_name() == "Brian");
  CHECK(search_r.unwrap()._test_age() == 25);
  CHECK(search_r.unwrap().key() == 0);
}

TEST_CASE("StudentTable Search #2") {
  StudentTable table;
  std::vector<StudentTuple> students;

  // Fill table to capacity
  for (size_t i = 0; i < StudentTable::Rows; i++) {
    StudentTuple student = {"Student" + std::to_string(i),
                            static_cast<tbl_row_t>(20 + i), 0};
    auto result = table.trait_insert(student);
    CHECK(result);
    CHECK(result.unwrap().key() == i);
    students.push_back(result.unwrap());
  }

  CHECK(table.trait_full());
  CHECK(table.trait_size() == StudentTable::Rows);

  // Remove last student
  CHECK(table.trait_remove(StudentTable::Rows - 1) == StudentTableError::None);
  CHECK(!table.trait_full());
  CHECK(table.trait_size() == StudentTable::Rows - 1);

  // Insert new student
  StudentTuple new_student = {"Student" + std::to_string(StudentTable::Rows),
                              static_cast<tbl_row_t>(20 + StudentTable::Rows),
                              0};
  auto result = table.trait_insert(new_student);
  CHECK(result);
  CHECK(result.unwrap().key() == StudentTable::Rows);
  CHECK(table.trait_full());
  CHECK(table.trait_size() == StudentTable::Rows);
  auto search_r = table.search(StudentTable::Rows);
  CHECK(search_r);
  CHECK(search_r.unwrap()._test_name() == "Student10");
}

TEST_CASE("StudentTable Search Not Found #2") {
  StudentTable table;
  auto search_r = table.search(0);
  CHECK(search_r.getError() == StudentTableError::NotFound);
}

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
  CHECK(table._test_s_avail_cnt() == 1);
  CHECK(table._test_x_avail_cnt() == 1);
  CHECK(id_r.unwrap().key() == 0);
  CHECK(id_r.unwrap()._test_age() == 25);
  CHECK(id_r.unwrap()._test_name() == "Brian");
  CHECK(!table.full());
}

TEST_CASE("Table Insert #2") {
  StudentTable table;
  StudentTuple student = {"Brian", 25, 42};
  CHECK(table.insert(student));
  CHECK(table._test_s_avail_cnt() == 1);
  CHECK(table._test_x_avail_cnt() == 1);
  CHECK(!table.empty());
  CHECK(!table.full());
  CHECK(table.insert(student));
  CHECK(table._test_s_avail_cnt() == 2);
  CHECK(table._test_x_avail_cnt() == 2);
  CHECK(!table.empty());
  CHECK(!table.full());
  CHECK(table.insert(student));
  CHECK(table._test_s_avail_cnt() == 3);
  CHECK(table._test_x_avail_cnt() == 3);
  CHECK(!table.empty());
  CHECK(!table.full());
  CHECK(table.insert(student));
  CHECK(table._test_s_avail_cnt() == 4);
  CHECK(table._test_x_avail_cnt() == 4);
  CHECK(!table.empty());
  CHECK(!table.full());
  CHECK(table.insert(student));
  CHECK(table._test_s_avail_cnt() == 5);
  CHECK(table._test_x_avail_cnt() == 5);
  CHECK(!table.empty());
  CHECK(!table.full());
  CHECK(table.insert(student));
  CHECK(table._test_s_avail_cnt() == 6);
  CHECK(table._test_x_avail_cnt() == 6);
  CHECK(!table.empty());
  CHECK(!table.full());
  CHECK(table.insert(student));
  CHECK(table._test_s_avail_cnt() == 7);
  CHECK(table._test_x_avail_cnt() == 7);
  CHECK(!table.empty());
  CHECK(!table.full());
  CHECK(table.insert(student));
  CHECK(table._test_s_avail_cnt() == 8);
  CHECK(table._test_x_avail_cnt() == 8);
  CHECK(!table.empty());
  CHECK(!table.full());
  CHECK(table.insert(student));
  CHECK(table._test_s_avail_cnt() == 9);
  CHECK(table._test_x_avail_cnt() == 9);
  CHECK(!table.empty());
  CHECK(!table.full());
  CHECK(table.insert(student));
  CHECK(table._test_s_avail_cnt() == 10);
  CHECK(table._test_x_avail_cnt() == 10);
  CHECK(!table.empty());
  CHECK(table.full());
}

TEST_CASE("StudentTable Insert #3") {
  StudentTable table;
  std::vector<StudentTuple> students;

  // Fill table to capacity
  for (size_t i = 0; i < StudentTable::Rows; i++) {
    StudentTuple student = {"Student" + std::to_string(i),
                            static_cast<tbl_row_t>(20 + i), 0};
    auto result = table.trait_insert(student);
    CHECK(result);
    CHECK(result.unwrap().key() == i);
    students.push_back(result.unwrap());
  }

  CHECK(table.trait_full());
  CHECK(table.trait_size() == StudentTable::Rows);

  // Remove last student
  CHECK(table.trait_remove(StudentTable::Rows - 1) == StudentTableError::None);
  CHECK(!table.trait_full());
  CHECK(table.trait_size() == StudentTable::Rows - 1);

  // Insert new student
  StudentTuple new_student = {"Student" + std::to_string(StudentTable::Rows),
                              static_cast<tbl_row_t>(20 + StudentTable::Rows),
                              0};
  auto result = table.trait_insert(new_student);
  CHECK(result);
  CHECK(result.unwrap().key() == StudentTable::Rows);
  CHECK(table.trait_full());
  CHECK(table.trait_size() == StudentTable::Rows);
}

TEST_CASE("Table Lock #1") {
  StudentTable table;
  StudentTuple student = {"Brian", 25, 0};
  auto insert_result = table.insert(student);
  CHECK(table._test_s_avail_cnt() == 1);
  CHECK(table._test_x_avail_cnt() == 1);
  CHECK(insert_result);
  auto wait_r = table.wait_shared();
  CHECK(wait_r);
  CHECK(wait_r.unwrap() == student);
  CHECK(table.size() == 1);
  CHECK(table._test_s_avail_cnt() == 1);
  CHECK(table._test_x_avail_cnt() == 0);
  auto wait_r2 = table._test_get_entry(0);
  CHECK(wait_r2);
  CHECK(wait_r2.unwrap().lock_ == 1);
  CHECK(table._test_s_avail_cnt() == 1);
  CHECK(table._test_x_avail_cnt() == 0);
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
  CHECK(table._test_s_avail_cnt() == 0);
  CHECK(table._test_x_avail_cnt() == 0);
  auto get_r = table._test_get_entry(0);
  CHECK(get_r);
  CHECK(get_r.unwrap().lock_ == -1);
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
  CHECK(table._test_s_avail_cnt() == 1);
  CHECK(table._test_x_avail_cnt() == 0);
  auto wait_r2 = table.wait_exclusive();
  CHECK(wait_r2.getError() == TupleTableError::Timeout);
  CHECK(table._test_s_avail_cnt() == 1);
  CHECK(table._test_x_avail_cnt() == 0);
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
  CHECK(table._test_s_avail_cnt() == 0);
  CHECK(table._test_x_avail_cnt() == 0);
  auto wait_r2 = table.wait_shared();
  CHECK(wait_r2.getError() == TupleTableError::Timeout);
  CHECK(table._test_s_avail_cnt() == 0);
  CHECK(table._test_x_avail_cnt() == 0);
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
  CHECK(table._test_s_avail_cnt() == 1);
  CHECK(table._test_x_avail_cnt() == 0);
  auto wait_r2 = table.wait_shared();
  CHECK(wait_r2);
  CHECK(table._test_s_avail_cnt() == 1);
  CHECK(table._test_x_avail_cnt() == 0);
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
  CHECK(table._test_s_avail_cnt() == 1);
  CHECK(table._test_x_avail_cnt() == 0);
  auto get_r = table._test_get_entry(0);
  CHECK(get_r);
  CHECK(get_r.unwrap().lock_ == 2);
  table.release(0);
  CHECK(table._test_s_avail_cnt() == 1);
  CHECK(table._test_x_avail_cnt() == 0);
  CHECK(get_r.unwrap().lock_ == 1);
  table.release(0);
  CHECK(table._test_s_avail_cnt() == 1);
  CHECK(table._test_x_avail_cnt() == 1);
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
  CHECK(table._test_s_avail_cnt() == 0);
  CHECK(table._test_x_avail_cnt() == 0);
  auto get_r = table._test_get_entry(0);
  CHECK(get_r);
  CHECK(get_r.unwrap().lock_ == -1);
  table.release(0);
  CHECK(get_r.unwrap().lock_ == 0);
  CHECK(table._test_s_avail_cnt() == 1);
  CHECK(table._test_x_avail_cnt() == 1);
  auto wait_r3 = table.wait_shared();
  CHECK(wait_r3);
  CHECK(table._test_s_avail_cnt() == 1);
  CHECK(table._test_x_avail_cnt() == 0);
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
  CHECK(table._test_s_avail_cnt() == 1);
  CHECK(table._test_x_avail_cnt() == 1);
  auto wait_r = table.wait_exclusive();
  CHECK(wait_r);
  CHECK(insert_result);
  table.remove(0);
  CHECK(table._test_s_avail_cnt() == 0);
  CHECK(table._test_x_avail_cnt() == 0);
  CHECK(table.empty());
}

TEST_CASE("StudentTable Remove #2") {
  StudentTable table;
  std::vector<StudentTuple> students;

  // Fill table to capacity
  for (size_t i = 0; i < StudentTable::Rows; i++) {
    StudentTuple student = {"Student" + std::to_string(i),
                            static_cast<tbl_row_t>(20 + i), 0};
    auto result = table.trait_insert(student);
    CHECK(result);
    CHECK(result.unwrap().key() == i);
    students.push_back(result.unwrap());
  }

  CHECK(table.trait_full());
  CHECK(table.trait_size() == StudentTable::Rows);

  // Remove last student
  CHECK(table.trait_remove(StudentTable::Rows - 1) == StudentTableError::None);
  CHECK(!table.trait_full());
  CHECK(table.trait_size() == StudentTable::Rows - 1);

  // Insert new student
  StudentTuple new_student = {"Student" + std::to_string(StudentTable::Rows),
                              static_cast<tbl_row_t>(20 + StudentTable::Rows),
                              0};
  auto result = table.trait_insert(new_student);
  CHECK(result);
  CHECK(result.unwrap().key() == StudentTable::Rows);
  CHECK(table.trait_full());
  CHECK(table.trait_size() == StudentTable::Rows);
  auto remove_r = table.trait_remove(StudentTable::Rows);
  CHECK(remove_r == StudentTableError::None);
}

TEST_CASE("StudentTable Write #1") {
  StudentTable table;
  StudentTuple student = {"Brian", 25, 0};
  auto insert_result = table.insert(student);
  CHECK(insert_result);
  auto wait_r = table.wait_exclusive();
  CHECK(wait_r);
  auto get_r = table._test_get_entry(0);
  CHECK(get_r);
  CHECK(get_r.unwrap().lock_ == -1);
  CHECK(table.write({"David", 30, 0}) == StudentTableError::None);
  CHECK(table._test_get_entry(0).unwrap().tuple_._test_name() == "David");
  CHECK(table._test_get_entry(0).unwrap().lock_ == -1);
  CHECK(table._test_get_entry(0).unwrap().tuple_._test_age() == 30);
}

TEST_CASE("StudentTable Clear #1") {
  StudentTable table;
  StudentTuple student = {"Brian", 25, 0};
  auto insert_result = table.insert(student);
  CHECK(insert_result);
  auto wait_r = table.wait_exclusive();
  CHECK(wait_r);
  table.clear();
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
  CHECK(table._test_s_avail_cnt() == 1);
  CHECK(table._test_x_avail_cnt() == 0);
  CHECK(table.promote(0) == TupleTableError::None);
  CHECK(get_r.unwrap().lock_ == -1);
  CHECK(table._test_s_avail_cnt() == 0);
  CHECK(table._test_x_avail_cnt() == 0);
}

TEST_CASE("Table Lock Promotion #3") {
  StudentTable table;
  StudentTuple student = {"Brian", 25, 0};
  auto table_r = table.insert(student);
  CHECK(table_r);
  auto wait1_r = table.wait_shared();
  auto wait2_r = table.wait_shared();
  CHECK(table._test_s_avail_cnt() == 1);
  CHECK(table._test_x_avail_cnt() == 0);
  CHECK(wait1_r);
  CHECK(wait2_r);
  auto get_r = table._test_get_entry(0);
  CHECK(get_r);
  CHECK(get_r.unwrap().lock_ == 2);
  CHECK(table.promote(0) == TupleTableError::Timeout);
  CHECK(table._test_s_avail_cnt() == 1);
  CHECK(table._test_x_avail_cnt() == 0);
}

}  // namespace PawnDB
