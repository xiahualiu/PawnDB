add_executable(lock-table-test main.cpp)
target_link_libraries(lock-table-test PRIVATE PawnDB::pawndb doctest)

doctest_discover_tests(lock-table-test)