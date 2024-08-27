#include <pawndb/user/fixed_string_type.h>

#include "doctest/doctest.h"

namespace FIXED_STRING_TYPE_TESTS {

  using namespace PawnDB;

  FixedString<20> A("Hello World!");

  TEST_CASE("Fixed String Constructor") {
    CHECK(A.to_std_string() == std::string_view("Hello World!"));
  }

  TEST_CASE("Fixed String Check Sum") {
    FixedString<20> B("Hello World!");
    FixedString<20> C("Not Hello World!");
    CHECK(A.check_sum() == B.check_sum());
    CHECK(A.check_sum() != C.check_sum());
  }

  TEST_CASE("Fixed String Not Equal") {
    FixedString<20> B("Hello World!");
    FixedString<20> C("Not Hello World!");
    CHECK(A != C);
    CHECK(!(A != B));
  }

  TEST_CASE("Fixed String Equal") {
    FixedString<20> B("Hello World!");
    FixedString<20> C("Not Hello World!");
    CHECK(A == B);
    CHECK(!(A == C));
  }

}  // namespace FIXED_STRING_TYPE_TESTS