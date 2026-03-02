add_executable(buffer-table-test main.cpp)
target_link_libraries(buffer-table-test PRIVATE PawnDB::pawndb doctest)

doctest_discover_tests(buffer-table-test)