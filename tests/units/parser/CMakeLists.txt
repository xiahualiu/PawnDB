add_executable(parser-test main.cpp)
target_link_libraries(parser-test PRIVATE PawnDB::pawndb doctest)

doctest_discover_tests(parser-test)