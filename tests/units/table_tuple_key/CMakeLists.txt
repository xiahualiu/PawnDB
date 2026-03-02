add_executable(table-tuple-key-test main.cpp)
target_link_libraries(table-tuple-key-test PRIVATE PawnDB::pawndb doctest)

doctest_discover_tests(table-tuple-key-test)