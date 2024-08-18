# --- cmake minimum version ---
cmake_minimum_required(VERSION 3.14)

# --- Standalone App ---
project(pawndb_app LANGUAGES CXX)

# ---- Create standalone executable ----
file(GLOB app_sources CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp)

# --- Depended on the PawnDB project ---
add_executable(${PROJECT_NAME} ${app_sources})

set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 17 OUTPUT_NAME "pawndb_app")

target_link_libraries(${PROJECT_NAME} PUBLIC pawndb)
