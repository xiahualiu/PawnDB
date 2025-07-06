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

# Export compile commands for tools like clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

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

# --- Options ---
option(PAWNDB_ENABLE_DOXYGEN "Build documentation with Doxygen" OFF)
option(PAWNDB_ENABLE_STYLE "Enable style checks" OFF)
option(PAWNDB_ENABLE_TEST "Enable testing targets" OFF)

# --- Include standalone and test ---
enable_testing()
add_subdirectory("source")
add_subdirectory("standalone")

if (PAWNDB_ENABLE_TEST)
  add_subdirectory("test")
endif()

if (PAWNDB_ENABLE_DOXYGEN)
  add_subdirectory("docs")
endif()

if (PAWNDB_ENABLE_STYLE)
  add_subdirectory("style")
endif()
