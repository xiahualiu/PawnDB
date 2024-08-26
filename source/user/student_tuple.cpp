#include <pawndb/user/fixed_point_type.h>
#include <pawndb/user/student_tuple.h>
#include <pawndb/user/u32_type.h>

using namespace PawnDB;

const static std::uint32_t MAX_STUDENT_ID = 100u;

StudentErrorEnum StudentTuple::validate() const {
  U32Int id = std::get<0>(list);
  if (id > MAX_STUDENT_ID) return StudentErrorEnum::ID_OUT_OF_RANGE;

  const FixedPoint<100> &grade = std::get<2>(list);
  if (grade > FixedPoint<100>(1, 10000))
    return StudentErrorEnum::GRADE_TOO_HIGH;
  else if (grade < FixedPoint<100>(0, 0))
    return StudentErrorEnum::GRADE_TOO_LOW;

  return StudentErrorEnum::NO_ERRORS;
}