add_executable(commit-table-test main.cpp)
target_link_libraries(commit-table-test PRIVATE PawnDB::pawndb doctest)

doctest_discover_tests(commit-table-test)