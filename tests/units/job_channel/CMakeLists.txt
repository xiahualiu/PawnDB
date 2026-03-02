add_executable(job-channel-test main.cpp)
target_link_libraries(job-channel-test PRIVATE PawnDB::pawndb doctest)

doctest_discover_tests(job-channel-test)