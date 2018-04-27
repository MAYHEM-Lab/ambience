include (CMakeForceCompiler)

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR xtensa)

CMAKE_FORCE_C_COMPILER(/home/fatih/esp-open-sdk/xtensa-lx106-elf/bin/xtensa-lx106-elf-gcc GNU)
CMAKE_FORCE_CXX_COMPILER(/home/fatih/esp-open-sdk/xtensa-lx106-elf/bin/xtensa-lx106-elf-g++ GNU)
#set(CMAKE_C_COMPILER xtensa-lx106-elf-gcc)
#set(CMAKE_CXX_COMPILER xtensa-lx106-elf-g++)
set(CMAKE_SYSROOT "sysroot")

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -nostdlib -Wall -Wextra -ffreestanding")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mlongcalls -nostdlib -ffunction-sections -fdata-sections -std=c++14 -Wall -Wextra -fno-unwind-tables -ffreestanding -fno-exceptions -fno-rtti -fno-threadsafe-statics")
set(CMAKE_EXE_LINKER_FLAGS "-Wl,--gc-sections -Wl,-EL -L/home/fatih/esp-open-sdk/sdk/ld -L/home/fatih/esp-open-sdk/sdk/lib -Teagle.app.v6.ld")