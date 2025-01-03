# --- cmake minimum version ---
cmake_minimum_required(VERSION 3.14)

# --- Standalone App ---
project(pawndb-app LANGUAGES CXX)

# ---- Create standalone executable ----
file(GLOB app_sources CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp)

# --- Depended on the PawnDB project ---
add_executable(${PROJECT_NAME} ${app_sources})

target_link_libraries(${PROJECT_NAME} PUBLIC ${CMAKE_PROJECT_NAME})
