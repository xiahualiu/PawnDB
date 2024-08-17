# Test project name
project(PawnDB_Tests LANGUAGES CXX)

# --- Add Doctest ---
add_subdirectory(doctest)

# ---- Options ----
option(TEST_INSTALLED_VERSION "Test the installed version" OFF)

# ---- Create binary ----
file(GLOB test_sources CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp)

add_executable(${PROJECT_NAME} ${test_sources})

set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 17)

# Include and link from system path
if(NOT TEST_INSTALLED_VERSION)
  target_link_libraries(${PROJECT_NAME} PRIVATE PawnDB doctest)
else()
  target_link_libraries(${PROJECT_NAME} PRIVATE doctest)
endif()

# ---- Add GreeterTests ----
enable_testing()
