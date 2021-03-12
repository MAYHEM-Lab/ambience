cmake_minimum_required(VERSION 3.13 FATAL_ERROR)

set(TOS_PROJECT_ROOT ${CMAKE_CURRENT_SOURCE_DIR})

list(APPEND CMAKE_MODULE_PATH "${TOS_PROJECT_ROOT}/cmake/modules")
list(APPEND CMAKE_MODULE_PATH "${TOS_PROJECT_ROOT}/cmake")
list(APPEND CMAKE_MODULE_PATH "${TOS_PROJECT_ROOT}/libs/ubench")
list(APPEND CMAKE_MODULE_PATH "${TOS_PROJECT_ROOT}/libs/lidlrt")
list(APPEND CMAKE_MODULE_PATH "${TOS_PROJECT_ROOT}/config")

include(HandleConfig)

project(tos VERSION 0.1.0)

option(BUILD_TESTS "Build unit tests, only works on hosted x86" OFF)
option(BUILD_EXAMPLES "Build examples" ON)
option(ENABLE_LTO "Enable Link Time Optimizations" OFF)
option(ENABLE_UBSAN "Enable undefined behavior sanitizer when available" ON)

include(Tos)
include(TosCompiler)
default_include_dirs()

if (BUILD_TESTS)
    include(CTest)
endif()

add_subdirectory(config/boards EXCLUDE_FROM_ALL)
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
