#include <pawndb/fixed_point.h>
#include <pawndb/tables.h>
#include <pawndb/tuples.h>

#include <iostream>

auto main() -> int {
  using namespace PawnDB;

  std::cout << "This is the standalone app." << std::endl;
  std::cout << "Size of fixed point: " << sizeof(FixedPoint) << std::endl;

  Tuple<FixedPoint, FixedPoint> row;
  Table<Tuple<FixedPoint, FixedPoint>, 10> test_table;

  std::cout << "Size of tuple<FixedPoint, FixedPoint>: " << sizeof(row) << std::endl;
  std::cout << "Size of Table<Tuple<FixedPoint, FixedPoint>, 10: " << sizeof(test_table)
            << std::endl;

  return 0;
}
