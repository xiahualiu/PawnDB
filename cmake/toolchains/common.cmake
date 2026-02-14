# Enable AddressSanitizer and coverage for debug builds
add_compile_options("$<$<CONFIG:DEBUG>:-fsanitize=address>")
add_compile_options("$<$<CONFIG:DEBUG>:--coverage>")
add_link_options("$<$<CONFIG:DEBUG>:-fsanitize=address>")
add_link_options("$<$<CONFIG:DEBUG>:--coverage>")

# Set the build type to Debug by default
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()