add_executable(client-test main.cpp)

doctest_discover_tests(client-test)