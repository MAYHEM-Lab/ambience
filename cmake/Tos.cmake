function(print_size target)
    add_custom_command(
            TARGET ${target} POST_BUILD
            COMMAND ${CMAKE_SIZE} -A $<TARGET_FILE:${target}>
            COMMENT "Print size"
    )
endfunction()

set(TOS_FLAGS "-Wall -Wextra -Wshadow -Wnon-virtual-dtor -Wpedantic \
     -ffunction-sections -fdata-sections -ffreestanding -g -nostdlib")

set(TOS_C_FLAGS "${TOS_FLAGS} -U__STRICT_ANSI__")
set(TOS_CXX_FLAGS "${TOS_FLAGS} -fno-rtti -fno-exceptions \
    -fno-unwind-tables -fno-threadsafe-statics -std=c++14 \
    -fstack-usage -fno-unwind-tables")

set(TOS ON)

set(CMAKE_C_FLAGS ${TOS_C_FLAGS})
set(CMAKE_CXX_FLAGS ${TOS_CXX_FLAGS})
