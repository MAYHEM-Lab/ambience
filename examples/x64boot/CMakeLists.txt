if (NOT ${TOS_ARCH} MATCHES "x86_64" AND NOT ${TOS_ARCH} MATCHES "i386")
    return()
endif()

if (${TOS_ARCH} MATCHES "x86_64")
    add_executable(x64boot main.cpp)
    target_link_libraries(x64boot PRIVATE
            tos_core vga_text uart_16550 tos_dynamic_log
            tos_coro_bind
    )
    return()
endif()

include(AddIso)

add_i386_loader(i386-loader ${CMAKE_BINARY_DIR}/../cmake-build-barex64 x64boot)
add_iso(i386-iso i386-loader)