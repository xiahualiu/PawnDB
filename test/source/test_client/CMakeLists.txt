add_executable(client-test-1 test1.cpp)
add_executable(client-test-2 test1.cpp)

doctest_discover_tests(client-test-1)
doctest_discover_tests(client-test-2)