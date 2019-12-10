set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR xtensa)

set(TOOLS_BASE /opt/x-tools/tos-esp-sdk)
set(SDK_ROOT ${TOOLS_BASE})

find_program(CMAKE_C_COMPILER 
    xtensa-lx106-elf-gcc
    HINTS /opt/x-tools/xtensa-lx106-elf C:/x-tools/xtensa-lx106-elf/bin /opt/x-tools/tos-esp-sdk/xtensa-lx106-elf/bin
    DOC "ESP GCC")

find_program(CMAKE_CXX_COMPILER 
    xtensa-lx106-elf-g++
    HINTS /opt/x-tools/xtensa-lx106-elf C:/x-tools/xtensa-lx106-elf/bin /opt/x-tools/tos-esp-sdk/xtensa-lx106-elf/bin
    DOC "ESP GCC")

find_program(CMAKE_SIZE
    xtensa-lx106-elf-size
    HINTS /opt/x-tools/xtensa-lx106-elf C:/x-tools/xtensa-lx106-elf/bin /opt/x-tools/tos-esp-sdk/xtensa-lx106-elf/bin
    DOC "ESP Size")

find_program(CMAKE_OBJCOPY 
    xtensa-lx106-elf-objcopy
    HINTS /opt/x-tools/xtensa-lx106-elf C:/x-tools/xtensa-lx106-elf/bin /opt/x-tools/tos-esp-sdk/xtensa-lx106-elf/bin
    DOC "ESP Objcopy")

if (CMAKE_C_COMPILER-NOTFOUND)
    message(STATUS "Can't find the toolchain!")
endif()

set(CMAKE_OBJCOPY "${CMAKE_OBJCOPY}" CACHE STRING "OBJCOPY")

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

set(LX106_FLAGS "-DICACHE_FLASH -mtext-section-literals -mlongcalls -nostdlib -U__STRICT_ANSI__")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${LX106_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${LX106_FLAGS}")

set(TOS_PROVIDE_LIBCXX ON)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}" CACHE STRING "CFLAGS")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}" CACHE STRING "CXXFLAGS")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}" CACHE STRING "")
