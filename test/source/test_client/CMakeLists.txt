# Basic START TXN and Abort TXN tests
add_executable(client-test-1 test1.cpp)

# START TXN to full
add_executable(client-test-2 test2.cpp)

# Add 1 tuple test
add_executable(client-test-3 test3.cpp)

# Add 20 tuples test
add_executable(client-test-4 test4.cpp)

doctest_discover_tests(client-test-1)
doctest_discover_tests(client-test-2)
doctest_discover_tests(client-test-3)
doctest_discover_tests(client-test-4)