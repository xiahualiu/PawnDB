# --- cmake minimum version ---
cmake_minimum_required(VERSION 3.20)

# add this options before PROJECT keyword
set(CMAKE_DISABLE_SOURCE_CHANGES ON)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)

# ---- Main Project ----
project(
  pawndb
  VERSION 0.1
  LANGUAGES CXX)

# Set the standard to ISO C++17 without extensions
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build shared libraries by default
option(BUILD_SHARED_LIBS "Build using shared libraries" ON)

# ---- Create library ----
add_library(${PROJECT_NAME} "")

# include paths
target_include_directories(
  ${PROJECT_NAME} PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
                         $<INSTALL_INTERFACE:include/>)

# Install to system default path minimum
include(cmake/install.cmake)

# --- Configure ccache ---
find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
  set(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE_PROGRAM})
endif()

# --- Include standalone and test ---
if(I_AM_CI_MACHINE)
  add_subdirectory("source")
  if(CI_MACHINE_STYLE_TASK)
    add_subdirectory("style")
  endif()
  if(CI_MACHINE_DOXYGEN_TASK)
    add_subdirectory("docs")
  endif()
  if(CI_MACHINE_TEST_TASK)
    enable_testing()
    add_subdirectory("test")
  endif()
  if(CI_MACHINE_STANDALONE_TASK)
    add_subdirectory("standalone")
  endif()
else()
  enable_testing()
  add_subdirectory("source")
  add_subdirectory("standalone")
  add_subdirectory("test")
  add_subdirectory("docs")
  add_subdirectory("style")
endif()
