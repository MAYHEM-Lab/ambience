list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}")

if (TOS_BOARD)
    include(boards/${TOS_BOARD})
endif()

if (TOS_CPU)
    include(cpu/${TOS_CPU})
elseif()
    message(ERROR "can't determine CPU!")
endif()
