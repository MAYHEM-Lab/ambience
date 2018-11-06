function(print_size target)
    add_custom_command(
            TARGET ${target} POST_BUILD
            COMMAND ${CMAKE_SIZE} -A $<TARGET_FILE:${target}>
            COMMENT "Print size"
    )
endfunction()

set(TOS_FLAGS "-Wall -Wextra -Wnon-virtual-dtor -Wpedantic \
     -ffunction-sections -fdata-sections -ffreestanding -g -pedantic -freorder-functions \
        -Wno-unknown-pragmas")

set(TOS_LINKER_FLAGS "-fno-threadsafe-statics -freorder-functions")

set(TOS_FLAGS "${TOS_FLAGS} -fstack-usage")
set(TOS_LINKER_FLAGS "${TOS_LINKER_FLAGS} -Wl,--gc-sections -Xlinker -Map=output.map")

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    message(STATUS "Using gcc")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(TOS_LINKER_FLAGS "${TOS_LINKER_FLAGS} -Wl,-dead_strip")
endif()

set(TOS_C_FLAGS "${TOS_FLAGS} -U__STRICT_ANSI__")
set(TOS_CXX_FLAGS "${TOS_FLAGS} -fno-rtti -fno-exceptions \
    -fno-unwind-tables -fno-threadsafe-statics -std=c++14")

set(TOS ON)

set(CMAKE_C_FLAGS ${TOS_C_FLAGS})
set(CMAKE_CXX_FLAGS ${TOS_CXX_FLAGS})
set(CMAKE_EXE_LINKER_FLAGS ${TOS_LINKER_FLAGS})
