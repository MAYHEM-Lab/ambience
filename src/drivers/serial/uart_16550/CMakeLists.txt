if (NOT ${TOS_ARCH} MATCHES "x86")
    return()
endif()

add_library(uart_16550 uart_16550.cpp)
target_include_directories(uart_16550 PUBLIC include)
target_link_libraries(uart_16550 PUBLIC tos_drivers arch_x86_64)