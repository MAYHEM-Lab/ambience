cmake_minimum_required(VERSION 3.13 FATAL_ERROR)

if (NOT TOS_PROJECT_ROOT)
set(TOS_PROJECT_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
endif()
message(STATUS ${TOS_PROJECT_ROOT})

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
option(ENABLE_UBSAN "Enable undefined behavior sanitizer when available" OFF)
option(ENABLE_RTTI "Enable run time type information" OFF)

include(Tos)
include(TosCompiler)
default_include_dirs()

if (BUILD_TESTS)
    include(CTest)
endif()

add_subdirectory(${TOS_PROJECT_ROOT}/config/boards EXCLUDE_FROM_ALL)
add_subdirectory(${TOS_PROJECT_ROOT}/third_party EXCLUDE_FROM_ALL)
add_subdirectory(${TOS_PROJECT_ROOT}/libs)
add_subdirectory(${TOS_PROJECT_ROOT}/src)
add_subdirectory(${TOS_PROJECT_ROOT}/apps EXCLUDE_FROM_ALL)
add_subdirectory(${TOS_PROJECT_ROOT}/staging EXCLUDE_FROM_ALL)

if (BUILD_EXAMPLES)
    add_subdirectory(${TOS_PROJECT_ROOT}/examples)
endif()
if (BUILD_TESTS)
    enable_testing()
    add_subdirectory(${TOS_PROJECT_ROOT}/tests)
endif()

add_subdirectory(${TOS_PROJECT_ROOT}/ambience)