#include <pawndb/basic_tables.h>
#include <pawndb/user/fixed_point_type.h>
#include <pawndb/user/fixed_string_type.h>
#include <pawndb/user/student_table.h>
#include <pawndb/user/student_tuple.h>
#include <pawndb/user/u32_type.h>

#include <iostream>

auto main() -> int {
  using namespace PawnDB;

  std::cout << "This is the standalone app." << std::endl;

  StudentTuple temp(U32Int(100), FixedString<20>("Alice"), FixedPoint<100>(1, 9999));

  std::cout << "Length of Tuple name: " << std::get<1>(temp.list).length << std::endl;
  std::cout << "Size of U32Int(100): " << sizeof(U32Int) << std::endl;
  std::cout << "Size of FixedString<20>: " << sizeof(FixedString<20>) << std::endl;
  std::cout << "Size of FixedPoint<100>: " << sizeof(FixedPoint<100>) << std::endl;
  std::cout << "Size of StudentTuple: " << sizeof(temp) << std::endl;
  std::cout << "Size of StudentTable: " << sizeof(StudentTable) << std::endl;

  return 0;
}
