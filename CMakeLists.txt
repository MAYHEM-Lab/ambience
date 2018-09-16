cmake_minimum_required(VERSION 3.5)

set(TOS ON)

if(TOS_AVR)
    set(AVR_CPU atmega328p)
    set(CMAKE_TOOLCHAIN_FILE cmake/AvrToolchain.cmake)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DF_CPU=16000000UL")
elseif(TOS_ESP)
    set(CMAKE_TOOLCHAIN_FILE cmake/lx106Toolchain.cmake)
elseif(TOS_ARM)
    set(CMAKE_TOOLCHAIN_FILE cmake/ArmToolchain.cmake)
elseif(TOS_X86)
else()
    message(FATAL_ERROR "Target architecture cannot be determined!")
endif()

project(tos)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")

message(STATUS "${CMAKE_CXX_FLAGS}")

include(${CMAKE_SOURCE_DIR}/cmake/TosTools.cmake)

add_subdirectory(src)

if (IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/libs/libemsha")
    add_subdirectory(libs/libemsha)
endif()

add_subdirectory(libs/kthreads)
add_subdirectory(libs/posix)

if(TOS_AVR)
    add_subdirectory(examples/gpio)
    #add_subdirectory(examples/spi)
    add_subdirectory(examples/alarm)
    #add_subdirectory(examples/sd)
    add_subdirectory(examples/temp)
    add_subdirectory(examples/int)
    #add_subdirectory(examples/simple_portable)
    add_subdirectory(examples/nrf24)
elseif(TOS_ESP)
    if (IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/apps/tvm")
        #add_subdirectory(apps/tvm)
    endif()

    if (IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/apps/cspot_lite")
        add_subdirectory(apps/cspot_lite)
    endif()

    if (IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/libs/mqtt")
        add_subdirectory(libs/mqtt)
    endif()

    add_subdirectory(examples/esp_fault)
    add_subdirectory(examples/esp_lwip)
    add_subdirectory(examples/esp_conn)
    add_subdirectory(examples/esp_mqtt)
    add_subdirectory(examples/esp_ssl)

    add_subdirectory(libs/zmtp)
    add_subdirectory(libs/CWPack)
elseif(TOS_X86)
    add_subdirectory(examples/simple_portable)

    add_subdirectory(tests)
elseif(TOS_ARM)
    add_subdirectory(examples/nrf52)
endif()

set(include_dest "include/tos")
set(lib_dest "lib/tos")
