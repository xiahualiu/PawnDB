#include <pawndb/fixed_point.h>
#include <pawndb/u32_type.h>
#include <pawndb/tables.h>
#include <pawndb/tuples.h>
#include <pawndb/fixed_string_type.h>

#include <iostream>

auto main() -> int {
  using namespace PawnDB;

  std::cout << "This is the standalone app." << std::endl;
  std::cout << "Size of fixed point: " << sizeof(FixedPoint) << std::endl;
  std::cout << "Size of fixed point: " << sizeof(FixedPoint) << std::endl;
  std::cout << "Size of u32_type: " << sizeof(U32Int) << std::endl;

  Tuple<FixedPoint, FixedPoint> row;
  Table<Tuple<FixedPoint, FixedPoint>, 10> test_table;

  std::cout << "Size of tuple<FixedPoint, FixedPoint>: " << sizeof(row) << std::endl;
  std::cout << "Size of Table<Tuple<FixedPoint, FixedPoint>, 10: " << sizeof(test_table)
            << std::endl;

  FixedString<20> test_string("Hello World!");

  std::cout << "Test string: " << test_string.to_std_string() << std::endl;
  std::cout << "Test string size: " << test_string.to_std_string().length() << std::endl;
  std::cout << "Test string size2: " << std::string_view("Hello World!").length() << std::endl;

  return 0;
}
