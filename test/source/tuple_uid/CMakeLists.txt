cmake_minimum_required(VERSION 3.20)

add_executable(tuple_uid_test main.cpp)

target_link_libraries(tuple_uid_test PRIVATE ${PROJECT_NAME})

add_test(NAME tuple_uid_test COMMAND tuple_uid_test)
