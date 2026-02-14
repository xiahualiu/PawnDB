# ---- Create standalone executable ----
file(GLOB app_sources CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp)

add_executable(pawndb-app ${app_sources})

target_link_libraries(pawndb-app PRIVATE PawnDB::pawndb)
