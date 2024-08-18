#include <pawndb/fixed_point.h>

#include "doctest/doctest.h"

using namespace PawnDB;

FixedPoint A(0, 1, 2);

TEST_CASE("Fixed Point Constructor") {
  CHECK(A.sign == 0);
  CHECK(A.l_part == 1);
  CHECK(A.r_part == 2);
}

TEST_CASE("Fixed Point Check Sum") {
  FixedPoint B(0, 1, 2);
  CHECK(A.check_sum() == B.check_sum());
}