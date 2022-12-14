list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}")

if (NOT TOS_BOARD AND NOT TOS_CPU)
    message(FATAL_ERROR "Cannot configure the build system without setting either a board or a CPU")
endif()

if (TOS_BOARD)
    if (NOT EXISTS ${CMAKE_CURRENT_LIST_DIR}/boards/${TOS_BOARD})
        message(FATAL_ERROR "Board \"${TOS_BOARD}\" does not exist!")
    endif()

    message(STATUS "Building for board ${TOS_BOARD}")

    include(${CMAKE_CURRENT_LIST_DIR}/boards/${TOS_BOARD}/board.cmake)
endif()

if (TOS_CPU)
    include(cpu/${TOS_CPU})
elseif()
    message(FATAL_ERROR "TOS_CPU is not set!")
endif()

if (TOS_TOOLCHAIN)
    set(CMAKE_TOOLCHAIN_FILE ${TOS_PROJECT_ROOT}/cmake/${TOS_TOOLCHAIN}.cmake)
endif()
