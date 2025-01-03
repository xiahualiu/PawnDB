# --- Test target ---
project(pawndb-test LANGUAGES CXX)

# --- Add Doctest ---
add_subdirectory(doctest)
include(${CMAKE_CURRENT_SOURCE_DIR}/doctest/scripts/cmake/doctest.cmake)
link_libraries(${CMAKE_PROJECT_NAME} doctest)

add_subdirectory("source")

# ---- Find all test tartgets ----
function(get_test_targets dir var_name)
  get_property(
    subdirectories
    DIRECTORY ${dir}
    PROPERTY SUBDIRECTORIES)
  foreach(subdir ${subdirectories})
    get_test_targets(${subdir} ${var_name})
  endforeach()

  get_property(
    test_targets
    DIRECTORY ${dir}
    PROPERTY BUILDSYSTEM_TARGETS)
  foreach(test_target ${test_targets})
    get_target_property(test_target_type ${test_target} TYPE)
    if(${test_target_type} STREQUAL "EXECUTABLE")
      list(APPEND ${var_name} ${test_target})
    endif()
    set(${var_name}
        ${${var_name}}
        PARENT_SCOPE)
  endforeach()
endfunction()

set(test_targets "")
get_test_targets("${CMAKE_CURRENT_SOURCE_DIR}/source" test_targets)

add_custom_target(
  run-all-tests
  DEPENDS ${test_targets}
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  COMMAND ctest -j 0)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  set(GCOV_EXE gcov)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  set(GCOV_EXE \"llvm-cov${CLANG_VERSION_SUFFIX}\")
endif()

add_custom_target(
  show-test-coverage
  DEPENDS ${test_targets}
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  COMMAND gcovr
  --gcov-executable ${GCOV_EXE} 
  . 
  --filter include/ 
  --filter source/
)
