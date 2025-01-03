# --- Style target ---
project(pawndb-style LANGUAGES CXX)

# --- Find clang-format ---
find_program(CLANG_FORMAT_PROGRAM NAMES clang-format)

# Globbing all C++ files
file(GLOB_RECURSE all_sources CONFIGURE_DEPENDS 
  ${CMAKE_SOURCE_DIR}/include/*.h
  ${CMAKE_SOURCE_DIR}/source/*.cpp
  ${CMAKE_SOURCE_DIR}/standalone/source/*.cpp
  ${CMAKE_SOURCE_DIR}/test/source/*.cpp
)

add_custom_target(
  check-clang-format
  COMMAND ${CLANG_FORMAT_PROGRAM}
  -style=file:${CMAKE_SOURCE_DIR}/.clang-format
  --dry-run
  -Werror
  ${all_sources}
)

add_custom_target(
  apply-clang-format
  COMMAND ${CLANG_FORMAT_PROGRAM}
  -i
  -style=file:${CMAKE_SOURCE_DIR}/.clang-format
  ${all_sources}
)
