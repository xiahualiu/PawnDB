# --- Test target ---
project(pawndb-test LANGUAGES CXX)

# --- Add Doctest ---
add_subdirectory(doctest)
include(${CMAKE_CURRENT_SOURCE_DIR}/doctest/scripts/cmake/doctest.cmake)

add_subdirectory("source")

# ---- Find all test tartgets ----
function(get_test_targets dir var_name)
  get_property(subdirectories DIRECTORY ${dir} PROPERTY SUBDIRECTORIES)
  foreach(subdir ${subdirectories})
    get_test_targets(${subdir} ${var_name})
  endforeach()

  get_property(test_targets DIRECTORY ${dir} PROPERTY BUILDSYSTEM_TARGETS)
  foreach(test_target ${test_targets})
    get_target_property(test_target_type ${test_target} TYPE)
    if(${test_target_type} STREQUAL "EXECUTABLE")
      list(APPEND ${var_name} ${test_target})
    endif()
    set(${var_name} ${${var_name}} PARENT_SCOPE)
  endforeach()
 endfunction() 

 set(test_targets "")
 get_test_targets("${CMAKE_CURRENT_SOURCE_DIR}/source" test_targets)

 add_custom_target(run-all-tests DEPENDS ${test_targets} WORKING_DIRECTORY ${CMAKE_BINARY_DIR} COMMAND ctest -j 0)