add_executable(x64boot main.cpp)
target_link_libraries(x64boot PRIVATE
        tos_core vga_text uart_16550 tos_dynamic_log
        tos_virtio tos_virtio_block_dev tos_virtio_network_dev
        lwipcore tos_coro_bind
        ae_user_space
)

