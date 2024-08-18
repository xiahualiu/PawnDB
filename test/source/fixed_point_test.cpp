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

TEST_CASE("Fixed Point Not Equal") {
  FixedPoint B1(1, 1, 2);
  FixedPoint B2(0, 2, 2);
  FixedPoint B3(0, 1, 3);
  FixedPoint C(0, 1, 2);
  CHECK(A != B1);
  CHECK(A != B2);
  CHECK(A != B3);
  CHECK(!(A != C));
}

TEST_CASE("Fixed Point Equal") {
  FixedPoint B(0, 1, 2);
  FixedPoint C1(1, 1, 2);
  FixedPoint C2(0, 2, 2);
  FixedPoint C3(0, 1, 3);
  CHECK(A == B);
  CHECK(!(A == C1));
  CHECK(!(A == C2));
  CHECK(!(A == C3));
}