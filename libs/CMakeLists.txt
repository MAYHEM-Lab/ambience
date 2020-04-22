SUBDIRLIST(LIB_DIRS "${CMAKE_CURRENT_LIST_DIR}")

message(STATUS ${LIB_DIRS})

foreach(lib ${LIB_DIRS})
    if (EXISTS "${CMAKE_CURRENT_LIST_DIR}/${lib}/CMakeLists.txt")
        add_subdirectory("${lib}")
    else()
        message(WARNING "Library ${lib} has no CMakeLists.txt")
    endif()
endforeach()
