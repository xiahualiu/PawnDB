file(GLOB cur_sources CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

add_library(pawndb ${cur_sources})
add_library(PawnDB::pawndb ALIAS pawndb)

target_compile_features(pawndb PUBLIC cxx_std_17)

target_include_directories(
	pawndb
	PUBLIC
		$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
		$<INSTALL_INTERFACE:include>
)

set_target_properties(
	pawndb
	PROPERTIES
		VERSION ${PROJECT_VERSION}
		SOVERSION ${PROJECT_VERSION_MAJOR}
)

install(
	TARGETS pawndb
	ARCHIVE DESTINATION lib
	LIBRARY DESTINATION lib
	RUNTIME DESTINATION bin
)

install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/pawndb DESTINATION include)
