find_package(Doxygen REQUIRED OPTIONAL_COMPONENTS dot mscgen dia)

set(DOXYGEN_GENERATE_HTML YES)

doxygen_add_docs(
  doxygen ${CMAKE_SOURCE_DIR}/include/
  ${CMAKE_SOURCE_DIR}/source/
  ${CMAKE_SOURCE_DIR}/standalone/source/
  ${CMAKE_SOURCE_DIR}/test/source/
  COMMENT "Generate HTML documentation"
)