add_executable(x64boot main.cpp)
target_link_libraries(x64boot PRIVATE
        tos_core vga_text uart_16550 tos_dynamic_log
        tos_coro_bind
)
add_iso(x64boot-iso x64boot)