cmake_minimum_required(VERSION 3.10)
project(bearssl)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(bearssl "")
target_include_directories(bearssl PUBLIC inc)

if (TOS)
    target_compile_definitions(bearssl PUBLIC TOS)
endif ()

add_library(BearSSL::BearSSL ALIAS bearssl)

add_subdirectory(src)
if (NOT TOS OR ${TOS_PLATFORM} MATCHES "x86")
    message(STATUS "Building tools")
    add_subdirectory(samples)
    add_subdirectory(tools)
    add_subdirectory(test)
endif ()