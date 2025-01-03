file(GLOB V_GLOB LIST_DIRECTORIES true "*")

foreach(V_FILE ${V_GLOB})
    if(IS_DIRECTORY ${V_FILE})
        add_subdirectory(${V_FILE})
    endif()
endforeach()