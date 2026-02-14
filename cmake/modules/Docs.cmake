# ---- Documentation tool discovery ----
find_package(Doxygen OPTIONAL_COMPONENTS dot mscgen dia)

# ---- Documentation output configuration ----
set(DOXYGEN_GENERATE_HTML YES)

# ---- Documentation target ----
if(DOXYGEN_FOUND)
  doxygen_add_docs(
    doxygen
    ${CMAKE_SOURCE_DIR}/include/
    ${CMAKE_SOURCE_DIR}/src/
    ${CMAKE_SOURCE_DIR}/apps/pawndb-cli/source/
    ${CMAKE_SOURCE_DIR}/tests/unit/
    COMMENT "Generate HTML documentation")
endif()