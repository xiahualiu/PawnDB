# --- cmake minimum version ---
cmake_minimum_required(VERSION 3.14)

# --- Test target ---
project(PawnDB_Tests LANGUAGES CXX)

# --- Add Doctest ---
add_subdirectory(doctest)

# ---- Options ----
option(TEST_INSTALLED_VERSION "Test the installed version" OFF)

file(GLOB test_sources CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp)

# ---- Create binary ----
add_executable(${PROJECT_NAME} ${test_sources})
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 17)

# Include and link from system path
if(NOT TEST_INSTALLED_VERSION)
  target_link_libraries(${PROJECT_NAME} PRIVATE PawnDB doctest)
else()
  target_link_libraries(${PROJECT_NAME} PRIVATE doctest)
endif()

# ---- Configure CTest ----
enable_testing()

include(${CMAKE_CURRENT_SOURCE_DIR}/doctest/scripts/cmake/doctest.cmake)

doctest_discover_tests(${PROJECT_NAME})