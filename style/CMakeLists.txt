# --- cmake minimum version ---
cmake_minimum_required(VERSION 3.14)

# --- Style target ---
project(pawndb_style LANGUAGES CXX)

# --- Find clang-format ---
find_program(CLANG_FORMAT_PROGRAM NAMES clang-format)

# Globbing all C++ files
file(GLOB_RECURSE all_sources CONFIGURE_DEPENDS 
  ${CMAKE_SOURCE_DIR}/include/*
  ${CMAKE_SOURCE_DIR}/source/*
  ${CMAKE_SOURCE_DIR}/standalone/source/*
  ${CMAKE_SOURCE_DIR}/test/source/*
)

add_custom_target(
  check-clang-format
  ALL
  COMMAND clang-format
  -style=file:${CMAKE_SOURCE_DIR}/.clang-format
  --dry-run
  -Werror
  ${all_sources}
)

add_custom_target(
  apply-clang-format
  ALL
  COMMAND clang-format
  -i
  -style=file:${CMAKE_SOURCE_DIR}/.clang-format
  ${all_sources}
)
