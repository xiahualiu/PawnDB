#include <pawndb/user/fixed_point_type.h>

#include "doctest/doctest.h"

namespace FIXED_POINT_TESTS {

  using namespace PawnDB;

  FixedPoint<100> A(1, 1);
  FixedPoint<100> A0(1, 0);

  TEST_CASE("Fixed Point Check Sum") {
    FixedPoint<100> B(1, 1);
    CHECK(A.verify());
    CHECK(B.verify());
  }

  TEST_CASE("Fixed Point Equal & Not Equal") {
    FixedPoint<100> B1(1, 1);
    FixedPoint<100> B2(0, 0);
    CHECK(A == B1);
    CHECK(A0 != B1);
    CHECK(A0 == B2);
    CHECK(A != B2);
  }

  TEST_CASE("Fixed Point > and <") {
    FixedPoint<100> B1(1, 1);
    FixedPoint<100> B2(0, 0);
    FixedPoint<100> B3(0, 100);
    CHECK(!(A0 < B2));
    CHECK(!(A0 > B2));
    CHECK(A0 < B1);
    CHECK(A > B3);
    CHECK(B3 < A);
    CHECK(B3 < B2);
    CHECK(B2 > B3);
  }

  TEST_CASE("Fixed Point print") {
    FixedPoint<100> B1(1, 1);
    FixedPoint<100> B2(0, 0);
    FixedPoint<100> B3(0, 100);
    CHECK(A.print() == "+0.01");
    CHECK(A0.print() == "+0.00");
    CHECK(B1.print() == "+0.01");
    CHECK(B3.print() == "-1.00");
  }

}  // namespace FIXED_POINT_TESTS