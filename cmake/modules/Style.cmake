# ---- Style tool discovery ----
find_program(CLANG_FORMAT_PROGRAM NAMES clang-format clang-format-20)

if(NOT CLANG_FORMAT_PROGRAM)
  message(WARNING "clang-format not found; style targets are not created")
  return()
endif()

# ---- Source set for style checks ----
file(GLOB_RECURSE all_sources CONFIGURE_DEPENDS
  ${CMAKE_SOURCE_DIR}/include/*.h
  ${CMAKE_SOURCE_DIR}/src/*.cpp
  ${CMAKE_SOURCE_DIR}/apps/pawndb-cli/source/*.cpp
  ${CMAKE_SOURCE_DIR}/tests/unit/*.cpp
)

# ---- Check formatting (non-mutating) ----
add_custom_target(
  check-clang-format
  COMMAND ${CLANG_FORMAT_PROGRAM}
  -style=file:${CMAKE_SOURCE_DIR}/.clang-format
  --dry-run
  -Werror
  ${all_sources}
)

# ---- Apply formatting (in-place) ----
add_custom_target(
  apply-clang-format
  COMMAND ${CLANG_FORMAT_PROGRAM}
  -i
  -style=file:${CMAKE_SOURCE_DIR}/.clang-format
  ${all_sources}
)