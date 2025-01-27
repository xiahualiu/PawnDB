cmake_minimum_required(VERSION 3.20)

# Set the C and C++ compilers to Clang-Cl
set(CMAKE_C_COMPILER "clang-cl${CLANG_VERSION_SUFFIX}")
set(CMAKE_CXX_COMPILER "clang-cl${CLANG_VERSION_SUFFIX}")

# Set the linker to MSVC's linker
set(CMAKE_LINKER "link.exe")

# Set the archiver to MSVC's lib.exe
set(CMAKE_AR "lib.exe")

# Set the ranlib if necessary (optional on Windows)
set(CMAKE_RANLIB "llvm-ranlib${CLANG_VERSION_SUFFIX}")

# Set the flags for Clang-Cl
add_compile_options(
  /W4
  /WX
  /permissive-
  /Zc:__cplusplus
)

# Enable AddressSanitizer and coverage for debug builds
add_compile_options("$<$<CONFIG:DEBUG>:/fsanitize=address>")
add_compile_options("$<$<CONFIG:DEBUG>:/coverage>")
add_link_options("$<$<CONFIG:DEBUG>:/fsanitize=address>")
add_link_options("$<$<CONFIG:DEBUG>:/coverage>")

# Set the build type to Debug by default
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()
