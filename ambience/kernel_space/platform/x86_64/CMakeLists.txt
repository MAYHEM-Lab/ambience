target_sources(aekernel PRIVATE
    x86_64.cpp
    acpi.cpp
)
target_link_libraries(aekernel PRIVATE
    uart_16550
    tos_elf
    vga_text
    tos_virtio
    tos_virtio_block_dev
    tos_virtio_network_dev
    tos_virtio_netif
    lwipcore
    lai
)

include(FileEmbed)

add_dependencies(aekernel sample_group)
embed_file(aekernel group1 $<TARGET_FILE:sample_group>.stripped)