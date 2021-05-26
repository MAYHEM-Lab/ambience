add_lidlc(kernel_schema schema.lidl)
add_lidl_dependency(kernel_schema filesystem_schema)
add_lidl_dependency(kernel_schema caplets_schema)
add_lidl_dependency(kernel_schema caplets_timeout_schema)

target_sources(
  aekernel
  PRIVATE x86_64.cpp
          acpi.cpp
          group_loading.cpp
          http_server.cpp
          pci.cpp
          platform_support.cpp
          page_allocator.cpp
)
target_link_libraries(
  aekernel
  PRIVATE uart_16550
          tos_elf
          vga_text
          tos_virtio
          tos_virtio_block_dev
          tos_virtio_network_dev
          tos_virtio_netif
          lwipcore
          lai
          virtio_blk_serv
          littlefs_server
          block_partition
          kernel_schema
)

include(FileEmbed)

add_dependencies(aekernel sample_group)
embed_file(aekernel group1 $<TARGET_FILE:sample_group>.stripped)
