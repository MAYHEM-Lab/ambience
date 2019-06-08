list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}")
list(APPEND CMAKE_MODULE_PATH "/home/fatih/tos")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/modules")

message(STATUS "hello")

include(config/HandleConfig)

include(Tos)

if (NOT ${CMAKE_BUILD_TYPE} MATCHES "MinSizeRel")
    message(WARNING "Build is not optimized for minimum size\nThis is not an error, but binaries will be large.")
endif()

if (TOS_TOOLCHAIN)
    include(cmake/${TOS_TOOLCHAIN})
endif()

set(TOS_BOARD "${TOS_BOARD}" CACHE STRING "BOARD")
set(TOS_CPU "${TOS_CPU}" CACHE STRING "CPU")
