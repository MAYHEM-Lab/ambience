set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR xtensa)

set(TOOLS_BASE /home/fatih/my_sdk)
set(SDK_ROOT ${TOOLS_BASE}/sdk)

set(CMAKE_C_COMPILER ${TOOLS_BASE}/xtensa-lx106-elf/bin/xtensa-lx106-elf-gcc)
set(CMAKE_CXX_COMPILER ${TOOLS_BASE}/xtensa-lx106-elf/bin/xtensa-lx106-elf-g++)
set(CMAKE_SIZE ${TOOLS_BASE}/xtensa-lx106-elf/bin/xtensa-lx106-elf-size)
set(CMAKE_SYSROOT "sysroot")

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} \
    -U__STRICT_ANSI__ -DICACHE_FLASH -mtext-section-literals -mlongcalls \
    -nostdlib -ffunction-sections -fdata-sections -Wall -Wextra -ffreestanding -fstack-usage")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mtext-section-literals -mlongcalls \
    -DICACHE_FLASH -nostdlib -ffunction-sections -fdata-sections -std=c++14 \
    -Wall -Wextra -fno-unwind-tables -ffreestanding -fno-exceptions -fno-rtti \
    -fno-threadsafe-statics -fstack-usage")

set(CMAKE_EXE_LINKER_FLAGS "-Wl,--gc-sections -Wl,-EL -L${SDK_ROOT}/ld -L${SDK_ROOT}/lib -Xlinker -Map=output.map")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}" CACHE STRING "CFLAGS")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}" CACHE STRING "CXXFLAGS")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}" CACHE STRING "")
