set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR xtensa)

include(FindToolchain)
find_gnu_toolchain(TRIPLE xtensa-lx106-elf SET)

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

set(LX106_FLAGS "-DICACHE_FLASH -mtext-section-literals -mlongcalls -nostdlib -U__STRICT_ANSI__")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${LX106_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${LX106_FLAGS}")

set(TOS_PROVIDE_LIBCXX OFF)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}" CACHE STRING "CFLAGS")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}" CACHE STRING "CXXFLAGS")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}" CACHE STRING "")
