cmake_minimum_required(VERSION 3.20)

# Coverage tool used by gcovr's --gcov-executable
set(
	PAWNDB_GCOV_PROGRAM
	"gcov"
	CACHE STRING "gcov-compatible coverage tool executable name")

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

include(${CMAKE_CURRENT_LIST_DIR}/common.cmake)
