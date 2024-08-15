#include <doctest/doctest.h>
#include <pawndb/pawndb.h>
#include <pawndb/version.h>

#include <string>

TEST_CASE("PawnDB") {
  CHECK("Hello, Tests!" == "Hello, Tests!");
}

TEST_CASE("PawnDB version") {
  static_assert(std::string_view(PAWNDB_VERSION) == std::string_view("1.0"));
  CHECK(std::string(PAWNDB_VERSION) == std::string("1.0"));
}
