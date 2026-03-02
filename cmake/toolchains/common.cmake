# Enable AddressSanitizer and coverage for debug builds (opt-in)
option(PAWNDB_ENABLE_ASAN "Enable AddressSanitizer in Debug builds" OFF)
option(PAWNDB_ENABLE_COVERAGE "Enable coverage instrumentation in Debug builds" OFF)

if(PAWNDB_ENABLE_ASAN)
  add_compile_options("$<$<CONFIG:DEBUG>:-fsanitize=address>")
  add_link_options("$<$<CONFIG:DEBUG>:-fsanitize=address>")
endif()

if(PAWNDB_ENABLE_COVERAGE)
  add_compile_options("$<$<CONFIG:DEBUG>:--coverage>")
  add_link_options("$<$<CONFIG:DEBUG>:--coverage>")
endif()

# Set the build type to Debug by default
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()