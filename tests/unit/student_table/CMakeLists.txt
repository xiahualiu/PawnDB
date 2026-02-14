add_executable(student-table-test main.cpp)
target_link_libraries(student-table-test PRIVATE PawnDB::pawndb doctest)

doctest_discover_tests(student-table-test)