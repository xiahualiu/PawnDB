file(GLOB cur_sources CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

target_sources(${PROJECT_NAME} PRIVATE ${cur_sources})