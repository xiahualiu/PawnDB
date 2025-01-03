cmake_minimum_required(VERSION 3.20)

set(CLANG_VERSION_SUFFIX "-18")

# Use Ninja generator by default
set(CMAKE_GENERATOR "Ninja")

# Disable the C compiler
set(CMAKE_C_COMPILER "")

# Set the C++ compiler to Clang
set(CMAKE_CXX_COMPILER "clang++${CLANG_VERSION_SUFFIX}")

# Set the linker to use Clang
set(CMAKE_LINKER "clang${CLANG_VERSION_SUFFIX}")

# Set the archiver to use Clang
set(CMAKE_AR "llvm-ar${CLANG_VERSION_SUFFIX}")

# Set the ranlib to use Clang
set(CMAKE_RANLIB "llvm-ranlib${CLANG_VERSION_SUFFIX}")

# Set the flags for Clang
add_compile_options(
  -Weverything
  -Werror
  -Wno-c++98-compat
  -Wno-c++98-compat-pedantic
  -Wno-padded
  -Wno-switch-enum
  -Wno-unsafe-buffer-usage)

# Enable AddressSanitizer and coverage for debug builds
add_compile_options("$<$<CONFIG:DEBUG>:-fsanitize=address>")
add_compile_options("$<$<CONFIG:DEBUG>:--coverage>")
add_link_options("$<$<CONFIG:DEBUG>:-fsanitize=address>")
add_link_options("$<$<CONFIG:DEBUG>:--coverage>")

# Set the build type to Debug by default
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()
