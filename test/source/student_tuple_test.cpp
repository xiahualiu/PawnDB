#include <pawndb/user/fixed_point_type.h>
#include <pawndb/user/fixed_string_type.h>
#include <pawndb/user/student_tuple.h>
#include <pawndb/user/u32_type.h>

#include "doctest/doctest.h"

namespace STUDENT_TUPLE_TESTS {

  using namespace PawnDB;

  StudentTuple A(U32Int(100), FixedString<20>("Alice"), FixedPoint<100>(1, 9999));
  StudentTuple B(U32Int(101), FixedString<20>("Bob"), FixedPoint<100>(0, 0));
  StudentTuple C(U32Int(1), FixedString<20>("Sam"), FixedPoint<100>(0, 100));
  StudentTuple D(U32Int(1), FixedString<20>("Sam"), FixedPoint<100>(1, 10001));

  TEST_CASE("Student Tuple Test") {
    CHECK(A.validate() == StudentErrorEnum::NO_ERRORS);
    CHECK(B.validate() == StudentErrorEnum::ID_OUT_OF_RANGE);
    CHECK(C.validate() == StudentErrorEnum::GRADE_TOO_LOW);
    CHECK(D.validate() == StudentErrorEnum::GRADE_TOO_HIGH);
  }

}  // namespace STUDENT_TUPLE_TESTS