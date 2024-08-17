# --- cmake minimum version ---
cmake_minimum_required(VERSION 3.14)

# add this options before PROJECT keyword
set(CMAKE_DISABLE_SOURCE_CHANGES ON)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)

# ---- Main Project ----
project(
  PawnDB
  VERSION 0.1
  LANGUAGES CXX
)

# ---- Add source files ----
file(GLOB_RECURSE lib_headers CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h")
file(GLOB_RECURSE lib_sources CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp")

# ---- Create library ----
add_library(${PROJECT_NAME} STATIC ${lib_sources})

# C++ properties
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 17)

# Enforce stricter compilation
add_compile_options(-Wall -Wpedantic -Wextra -Werror)

# include paths
target_include_directories(
  ${PROJECT_NAME} PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
                         $<INSTALL_INTERFACE:include/>
)

# Set Public header property
set_target_properties(${PROJECT_NAME} PROPERTIES PUBLIC_HEADER ${lib_headers})

# Install to system default path minimum
string(TOLOWER "${PROJECT_NAME}" LOWERCASE_PROJECT_NAME)
install(
  TARGETS ${PROJECT_NAME}
  LIBRARY DESTINATION lib
  PUBLIC_HEADER DESTINATION include/${LOWERCASE_PROJECT_NAME}
)

# --- Configure cppcheck ---
find_program(CPPCHECK_PROGRAM NAMES cppcheck)
if(CPPCHECK_PROGRAM)
  set(CMAKE_CXX_CPPCHECK "cppcheck")
  list(APPEND CMAKE_CXX_CPPCHECK 
    "--inline-suppr"
    "--force"
    "--suppressions-list=${CMAKE_SOURCE_DIR}/CppCheckSuppressions.txt"
  )
endif()

# --- Configure ccache ---
find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
  set(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE_PROGRAM})
endif()

# --- Include standalone and test ---
add_subdirectory(standalone EXCLUDE_FROM_ALL)
add_subdirectory(test EXCLUDE_FROM_ALL)
add_subdirectory(style EXCLUDE_FROM_ALL)
