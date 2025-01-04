# --- Test target ---
project(pawndb-test LANGUAGES CXX)

# --- Add Doctest ---
add_subdirectory(doctest)
include(${CMAKE_CURRENT_SOURCE_DIR}/doctest/scripts/cmake/doctest.cmake)
link_libraries(${CMAKE_PROJECT_NAME} doctest)

add_subdirectory("source")

# ---- Find all test tartgets ----

# Initialize cache variable
set(ALL_TEST_TARGETS "" CACHE INTERNAL "")

function(get_test_targets dir)
  # Get subdirectories
  get_property(subdirectories
    DIRECTORY ${dir}
    PROPERTY SUBDIRECTORIES)
    
  # Process each subdirectory
  foreach(subdir ${subdirectories})
    get_test_targets(${subdir})
  endforeach()

  # Get test targets in current directory
  get_property(test_targets
    DIRECTORY ${dir}
    PROPERTY BUILDSYSTEM_TARGETS)
    
  # Add executable targets to cache variable
  foreach(test_target ${test_targets})
    get_target_property(test_target_type ${test_target} TYPE)
    if(${test_target_type} STREQUAL "EXECUTABLE")
      set(ALL_TEST_TARGETS ${ALL_TEST_TARGETS} ${test_target} CACHE INTERNAL "")
    endif()
  endforeach()
endfunction()

# Get all test targets
get_test_targets("${CMAKE_CURRENT_SOURCE_DIR}/source")

add_custom_target(
  run-all-tests
  DEPENDS ${ALL_TEST_TARGETS}
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  COMMAND ctest -j 0)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  set(GCOV_EXE gcov)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  set(GCOV_EXE \"llvm-cov${CLANG_VERSION_SUFFIX}\")
endif()

add_custom_target(
  show-test-coverage
  DEPENDS ${ALL_TEST_TARGETS}
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  COMMAND gcovr 
  --gcov-executable 
  "${GCOV_EXE} gcov" 
  . --filter include/ 
  --filter source/)

add_custom_target(clean-coverage
  COMMAND find ${CMAKE_BINARY_DIR} -name "*.gcda" -delete
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)
