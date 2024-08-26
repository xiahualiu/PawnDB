#ifndef STUDENT_TUPLE_H
#define STUDENT_TUPLE_H

#include <pawndb/basic_tuples.h>
#include <pawndb/user/fixed_point_type.h>
#include <pawndb/user/fixed_string_type.h>
#include <pawndb/user/u32_type.h>

#include <tuple>

namespace PawnDB {

  enum StudentErrorEnum {
    NO_ERRORS,
    ID_OUT_OF_RANGE,
    GRADE_TOO_LOW,
    GRADE_TOO_HIGH,
  };

  class StudentTuple : public BasicTuple<U32Int, FixedString<20>, FixedPoint<100>> {
  public:
    inline StudentTuple(U32Int&& id, FixedString<20>&& name, FixedPoint<100>&& grade) {
      list = std::make_tuple(id, name, grade);
    }

    StudentErrorEnum validate() const;

  private:
  };

}  // namespace PawnDB

#endif