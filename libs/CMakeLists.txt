SUBDIRLIST(LIB_DIRS "${CMAKE_CURRENT_LIST_DIR}")

message(STATUS ${LIB_DIRS})

foreach(lib ${LIB_DIRS})
    add_subdirectory("${lib}")
endforeach()
