include (CMakeForceCompiler)

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)

set(tools /opt/x-tools/gcc-arm-none-eabi-7-2017-q4-major)
set(CMAKE_SYSROOT "sysroot")

set(CMAKE_C_COMPILER ${tools}/bin/arm-none-eabi-gcc)
set(CMAKE_CXX_COMPILER ${tools}/bin/arm-none-eabi-g++)

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

set(TOS_NRF52_FLAGS "-DNRF52 -DFLOAT_ABI_HARD -DCONFIG_GPIO_AS_PINRESET -DNRF52832_XXAA -DNRF52_PAN_74 -mthumb -mabi=aapcs -mfloat-abi=hard -mfpu=fpv4-sp-d16 -u _printf_float -fno-builtin -fshort-enums")
set(TOS_COMMON_FLAGS "-mcpu=cortex-m4 -ffunction-sections -fdata-sections -Wall -Wextra -ffreestanding ${TOS_NRF52_FLAGS}")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${TOS_COMMON_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${TOS_COMMON_FLAGS} -std=c++14 -pedantic -fno-unwind-tables -fno-exceptions -fno-rtti -fno-threadsafe-statics")
set(CMAKE_EXE_LINKER_FLAGS "-Wl,--gc-sections -fno-threadsafe-statics ${TOS_NRF52_FLAGS}")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}" CACHE STRING "CFLAGS")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}" CACHE STRING "CXXFLAGS")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}" CACHE STRING "")

set(SDK_ROOT /home/fatih/Downloads/nRF5_SDK_15.0.0_a53641a)

include_directories(${SDK_ROOT}/components/toolchain/cmsis/include/)
include_directories(${SDK_ROOT}/integration/nrfx)
include_directories(${SDK_ROOT}/modules/nrfx)
include_directories(${SDK_ROOT}/modules/nrfx/mdk/)
include_directories(${SDK_ROOT}/modules/nrfx/hal)
include_directories(${SDK_ROOT}/modules/nrfx/drivers/include)
include_directories(${SDK_ROOT}/components/libraries/delay)
include_directories(${SDK_ROOT}/components/libraries/experimental_log)
include_directories(${SDK_ROOT}/components/libraries/experimental_log/src)
include_directories(${SDK_ROOT}/components/libraries/experimental_section_vars)
include_directories(${SDK_ROOT}/components/libraries/util)
include_directories(${SDK_ROOT}/components/drivers_nrf/nrf_soc_nosd)