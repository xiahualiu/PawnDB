add_executable(ret-channel-test main.cpp)
target_link_libraries(ret-channel-test PRIVATE PawnDB::pawndb doctest)

doctest_discover_tests(ret-channel-test)