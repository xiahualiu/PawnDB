# --- cmake minimum version ---
cmake_minimum_required(VERSION 3.14)

# --- Debug
#set(CMAKE_VERBOSE_MAKEFILE ON)

# add this options before PROJECT keyword
set(CMAKE_DISABLE_SOURCE_CHANGES ON)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)

# ---- Project ----
project(
  PawnDB
  VERSION 1.0
  LANGUAGES CXX
)

# ---- Add source files ----
file(GLOB_RECURSE lib_headers CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h")
file(GLOB_RECURSE lib_sources CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp")

# ---- Create library ----
add_library(${PROJECT_NAME} STATIC ${lib_sources})

# C++ properties
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 17)

# Enforce stricter compilation
add_compile_options(-Wall -Wpedantic -Wextra -Werror)

# include paths
target_include_directories(
  ${PROJECT_NAME} PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
                         $<INSTALL_INTERFACE:include/>
)

# Set Public header property
set_target_properties(${PROJECT_NAME} PROPERTIES PUBLIC_HEADER ${lib_headers})

# Install to system default path minimum
install(TARGETS ${PROJECT_NAME}
        LIBRARY DESTINATION lib
        PUBLIC_HEADER DESTINATION include)

# --- Include standalone and test ---
add_subdirectory(standalone)
add_subdirectory(test)
