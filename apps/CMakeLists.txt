SUBDIRLIST(APP_DIRS "${CMAKE_CURRENT_LIST_DIR}")

foreach(app ${APP_DIRS})
    message(STATUS "Building application \"${app}\"")
    add_subdirectory("${app}")
endforeach()
