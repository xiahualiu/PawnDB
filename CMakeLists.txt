# ---- CMake version ----
cmake_minimum_required(VERSION 3.20)

# ---- Source/build tree safeguards ----
set(CMAKE_DISABLE_SOURCE_CHANGES ON)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)

# ---- Project definition ----
project(
  pawndb
  VERSION 0.1
  LANGUAGES CXX)

# ---- Global language settings ----
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ---- Global build defaults ----
option(BUILD_SHARED_LIBS "Build using shared libraries" ON)

# ---- Build acceleration (optional) ----
find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
  set(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE_PROGRAM})
endif()

# ---- Feature toggles ----
option(PAWNDB_ENABLE_DOXYGEN "Build documentation with Doxygen" OFF)
option(PAWNDB_ENABLE_STYLE "Enable style checks" OFF)
option(PAWNDB_ENABLE_TEST "Enable testing targets" OFF)

# ---- Core targets ----
add_subdirectory("src")
add_subdirectory("apps/pawndb-cli")

# ---- Optional test/docs/style targets ----
if (PAWNDB_ENABLE_TEST)
  enable_testing()
  include(${CMAKE_SOURCE_DIR}/cmake/modules/Testing.cmake)
endif()

if (PAWNDB_ENABLE_DOXYGEN)
  include(cmake/modules/Docs.cmake)
endif()

if (PAWNDB_ENABLE_STYLE)
  include(cmake/modules/Style.cmake)
endif()

# ---- Package configuration + install metadata ----
include(cmake/package/package.cmake)
