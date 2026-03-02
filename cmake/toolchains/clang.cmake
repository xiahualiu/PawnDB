cmake_minimum_required(VERSION 3.20)

set(CLANG_VERSION "20")

# Coverage tool used by gcovr's --gcov-executable
set(
  PAWNDB_GCOV_PROGRAM
  "llvm-cov-${CLANG_VERSION}"
  CACHE STRING "gcov-compatible coverage tool executable name")

# Disable the C compiler
set(CMAKE_C_COMPILER "")

# Set the C++ compiler to Clang
set(CMAKE_CXX_COMPILER "clang++-${CLANG_VERSION}")

# Set the linker to use Clang
set(CMAKE_LINKER "clang-${CLANG_VERSION}")

# Set the archiver to use Clang
set(CMAKE_AR "llvm-ar-${CLANG_VERSION}")

# Set the ranlib to use Clang
set(CMAKE_RANLIB "llvm-ranlib-${CLANG_VERSION}")

# Set the flags for Clang
add_compile_options(
  -Weverything
  -Werror
  -Wno-c++98-compat
  -Wno-c++98-compat-pedantic
  -Wno-padded
  -Wno-switch-enum
  -Wno-unsafe-buffer-usage
  -Wno-exit-time-destructors)

include(${CMAKE_CURRENT_LIST_DIR}/common.cmake)
