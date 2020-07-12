cmake_minimum_required(VERSION 3.13 FATAL_ERROR)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/libs/ubench")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/libs/lidlrt")

include(config/HandleConfig.cmake)

project(tos VERSION 0.1.0)

option(BUILD_TESTS "Build unit tests, only works on hosted x86" OFF)
option(BUILD_EXAMPLES "Build examples" ON)
option(ENABLE_LTO "Enable Link Time Optimizations" ON)

include(Tos)

if (BUILD_TESTS)
    include(CTest)
endif()

add_subdirectory(libs)
add_subdirectory(src)
add_subdirectory(apps EXCLUDE_FROM_ALL)
add_subdirectory(third_party)

if (BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
if (BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
