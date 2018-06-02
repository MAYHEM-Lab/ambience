include (CMakeForceCompiler)

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)

set(tools /opt/x-tools/gcc-arm-none-eabi-7-2017-q4-major)
set(CMAKE_SYSROOT "sysroot")

CMAKE_FORCE_C_COMPILER(${tools}/bin/arm-none-eabi-gcc GNU)
CMAKE_FORCE_CXX_COMPILER(${tools}/bin/arm-none-eabi-g++ GNU)

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

set(CMAKE_C_FLAGS "-mcpu=cortex-m4 ${CMAKE_C_FLAGS} -Wall -Wextra -ffreestanding -fpic")
set(CMAKE_CXX_FLAGS "-mcpu=cortex-m4 ${CMAKE_CXX_FLAGS} -std=c++14 -Wall -Wextra -fno-unwind-tables -ffreestanding -fpic -fno-exceptions -fno-rtti")

include_directories(/Users/fatih/Downloads/nRF5_SDK_15.0.0_a53641a/components/toolchain/cmsis/include/)
include_directories(/Users/fatih/Downloads/nRF5_SDK_15.0.0_a53641a/modules/nrfx/mdk/)
