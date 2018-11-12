cmake_minimum_required(VERSION 3.5)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(${CMAKE_SOURCE_DIR}/cmake/Tos.cmake)

if(TOS_AVR)
    set(AVR_CPU atmega328p)
    set(CMAKE_TOOLCHAIN_FILE cmake/AvrToolchain.cmake)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DF_CPU=8000000UL -DTOS_DEFAULT_STACK_SIZE=256")
elseif(TOS_ESP)
    set(CMAKE_TOOLCHAIN_FILE cmake/lx106Toolchain.cmake)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DTOS_DEFAULT_STACK_SIZE=2048")
elseif(TOS_ARM)
    set(CMAKE_TOOLCHAIN_FILE cmake/nrf52Toolchain.cmake)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DTOS_DEFAULT_STACK_SIZE=2048")
elseif(TOS_STM32)
    set(CMAKE_TOOLCHAIN_FILE cmake/stm32Toolchain.cmake)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DTOS_DEFAULT_STACK_SIZE=2048")
elseif(TOS_X86)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DTOS_DEFAULT_STACK_SIZE=8192*2")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}" CACHE STRING "CXXFLAGS")
else()
    message(FATAL_ERROR "Target architecture cannot be determined!")
endif()


project(tos)
message(STATUS "${CMAKE_CXX_FLAGS}")

enable_testing()

add_subdirectory(src)

if (IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/libs/libemsha")
    add_subdirectory(libs/libemsha)
endif()

add_subdirectory(libs/kthreads)
add_subdirectory(libs/posix)
add_subdirectory(libs/caps)
add_subdirectory(libs/toscxx)
add_subdirectory(libs/sml)
add_subdirectory(libs/CWPack)

if(TOS_AVR)
    add_subdirectory(examples/gpio)
    #add_subdirectory(examples/spi)
    add_subdirectory(examples/alarm)
    #add_subdirectory(examples/sd)
    add_subdirectory(examples/temp)
    add_subdirectory(examples/int)
    add_subdirectory(examples/simple_portable)
    add_subdirectory(examples/nrf24)
    add_subdirectory(examples/xbee)
    add_subdirectory(examples/ina219)

    if (IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/apps/tvm")
        #add_subdirectory(apps/tvm)
    endif()
elseif(TOS_ESP)

    if (IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/apps/cspot_lite")
        add_subdirectory(apps/cspot_lite)
    endif()

    if (IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/libs/mqtt")
        add_subdirectory(libs/mqtt)
    endif()

    #add_subdirectory(examples/esp_fault)
    add_subdirectory(examples/esp_lwip)
    add_subdirectory(examples/esp_conn)
    #add_subdirectory(examples/esp_mqtt)
    #add_subdirectory(examples/esp_ssl)
    add_subdirectory(examples/esp_spi)
    add_subdirectory(examples/esp_adc)

    #add_subdirectory(examples/aws_push)

    add_subdirectory(examples/dht11_sample)

    add_subdirectory(libs/zmtp)
    add_subdirectory(libs/sntp)
elseif(TOS_X86)

    if (IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/apps/cspot_lite")
        add_subdirectory(apps/cspot_lite)
    endif()

    add_subdirectory(examples/simple_portable)

    add_subdirectory(tests)
elseif(TOS_ARM)
    add_subdirectory(examples/nrf52)
elseif(TOS_STM32)
    add_subdirectory(examples/stm32_blink)
endif()

set(include_dest "include/tos")
set(lib_dest "lib/tos")
