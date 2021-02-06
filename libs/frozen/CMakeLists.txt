cmake_minimum_required(VERSION 3.8)
#
# Here we check whether frozen is being configured in isolation or as a component
# of a larger proeject. To do so, we query whether the `PROJECT_NAME` CMake
# variable has been defined. In the case it has, we can conclude frozen is a
# subproject.
#
# This convention has been borrowed from the Catch C++ unit testing library.
#
if( DEFINED PROJECT_NAME )
    set( subproject ON )
else()
    set( subproject OFF )
endif()

project(frozen VERSION 1.0.0 LANGUAGES CXX)

add_library(frozen INTERFACE)
add_library(frozen::frozen ALIAS frozen)

target_compile_features(frozen INTERFACE cxx_std_14)
target_compile_definitions(frozen INTERFACE FROZEN_NO_EXCEPTIONS)

target_include_directories(frozen INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
        $<INSTALL_INTERFACE:include>)

add_subdirectory(include/frozen)
