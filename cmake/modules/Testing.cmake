# ---- Third-party test framework ----
if(NOT DEFINED DOCTEST_NO_INSTALL)
  set(DOCTEST_NO_INSTALL ON CACHE BOOL "Disable doctest installation for PawnDB builds")
endif()
add_subdirectory(${CMAKE_SOURCE_DIR}/tests/doctest ${CMAKE_BINARY_DIR}/tests-doctest)
include(${CMAKE_SOURCE_DIR}/tests/doctest/scripts/cmake/doctest.cmake)

# ---- Project unit tests ----
add_subdirectory(${CMAKE_SOURCE_DIR}/tests/units ${CMAKE_BINARY_DIR}/tests-units)

# ---- Convenience target: build test executables + run ctest ----
set(ALL_TEST_TARGETS "")

function(get_test_targets dir)
  get_property(subdirectories DIRECTORY ${dir} PROPERTY SUBDIRECTORIES)
  foreach(subdir ${subdirectories})
    get_test_targets(${subdir})
  endforeach()

  get_property(test_targets DIRECTORY ${dir} PROPERTY BUILDSYSTEM_TARGETS)
  foreach(test_target ${test_targets})
    get_target_property(test_target_type ${test_target} TYPE)
    if(test_target_type STREQUAL "EXECUTABLE")
      list(APPEND ALL_TEST_TARGETS ${test_target})
    endif()
  endforeach()

  set(ALL_TEST_TARGETS ${ALL_TEST_TARGETS} PARENT_SCOPE)
endfunction()

get_test_targets("${CMAKE_SOURCE_DIR}/tests/units")
list(REMOVE_DUPLICATES ALL_TEST_TARGETS)

add_custom_target(
  run-all-tests
  DEPENDS ${ALL_TEST_TARGETS}
  COMMAND ${CMAKE_CTEST_COMMAND} --test-dir ${CMAKE_BINARY_DIR} --output-on-failure
  USES_TERMINAL
)

# ---- Optional coverage report target ----
set(GCOV_EXE "")

if(DEFINED PAWNDB_GCOV_PROGRAM AND NOT PAWNDB_GCOV_PROGRAM STREQUAL "")
  if(PAWNDB_GCOV_PROGRAM MATCHES "^llvm-cov")
    set(GCOV_EXE "${PAWNDB_GCOV_PROGRAM} gcov")
  else()
    set(GCOV_EXE "${PAWNDB_GCOV_PROGRAM}")
  endif()
endif()

# Check if gcovr is available for generating coverage reports
find_program(GCOVR_PROGRAM gcovr)
if(GCOVR_PROGRAM)
  if(GCOV_EXE)
    add_custom_target(
      show-test-coverage
      WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
      COMMAND ${GCOVR_PROGRAM}
      --gcov-executable
      "${GCOV_EXE}"
      .
      --filter include/
      --filter src/
    )
  else()
    add_custom_target(
      show-test-coverage
      WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
      COMMAND ${GCOVR_PROGRAM}
      .
      --filter include/
      --filter src/
    )
  endif()
endif()

# ---- Cleanup target for coverage artifacts ----
add_custom_target(
  clean-coverage
  COMMAND find ${CMAKE_BINARY_DIR} -name "*.gcda" -delete
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)