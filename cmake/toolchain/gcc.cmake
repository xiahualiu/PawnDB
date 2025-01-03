cmake_minimum_required(VERSION 3.20)

# Use Ninja generator by default
set(CMAKE_GENERATOR "Ninja")

# Disable the C compiler
set(CMAKE_C_COMPILER "")

# Set the C++ compiler to g++
set(CMAKE_CXX_COMPILER "g++")

# Set the linker to use g++
set(CMAKE_LINKER "g++")

# Set the archiver to use GNU ar
set(CMAKE_AR "ar")

# Set the ranlib to use GNU ranlib
set(CMAKE_RANLIB "ranlib")

# Set the flags for g++
add_compile_options(-Wall -Wextra -Werror -Wno-class-memaccess)

# Enable AddressSanitizer and coverage for debug builds
add_compile_options("$<$<CONFIG:DEBUG>:-fsanitize=address>")
add_compile_options("$<$<CONFIG:DEBUG>:--coverage>")
add_link_options("$<$<CONFIG:DEBUG>:-fsanitize=address>")
add_link_options("$<$<CONFIG:DEBUG>:--coverage>")

# Set the build type to Debug by default
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()
