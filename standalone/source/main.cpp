#include <pawndb/fixed_point.h>
#include <pawndb/tuples.h>

#include <iostream>

auto main() -> int {
  using namespace PawnDB;

  std::cout << "This is the standalone app." << std::endl;
  std::cout << "Size of fixed point: " << sizeof(FixedPoint) << std::endl;

  Tuple<FixedPoint, FixedPoint> row;

  std::cout << "Size of tuple<FixedPoint, FixedPoint>: " << sizeof(row) << std::endl;

  return 0;
}
