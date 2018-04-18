set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR avr)

set(CMAKE_SYSROOT "sysroot")

set(CMAKE_C_COMPILER avr-gcc)
set(CMAKE_CXX_COMPILER avr-g++)

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

set(CMAKE_C_FLAGS "-mmcu=${AVR_CPU} ${CMAKE_C_FLAGS} -Wall -Wextra -ffreestanding")
set(CMAKE_CXX_FLAGS "-mmcu=${AVR_CPU} ${CMAKE_CXX_FLAGS} -ffunction-sections -fdata-sections -std=c++17 -Wall -Wextra -fno-unwind-tables -ffreestanding -fno-exceptions -fno-rtti -fno-threadsafe-statics")
set(CMAKE_EXE_LINKER_FLAGS "-Wl,--gc-sections")