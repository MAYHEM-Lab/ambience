add_lidlc(kernel_schema schema.lidl)
add_lidl_dependency(kernel_schema filesystem_schema)
add_lidl_dependency(kernel_schema caplets_schema)
add_lidl_dependency(kernel_schema caplets_timeout_schema)

target_sources(
  aekernel
  PRIVATE acpi.cpp
          http_server.cpp
          pci.cpp
          platform_support.cpp
          page_allocator.cpp
          apic.cpp
          machine_impl.cpp
)
target_link_libraries(
  aekernel
  PUBLIC  tos_board
          tos_virtio
          tos_virtio_block_dev
          tos_virtio_network_dev
          tos_virtio_netif
          lwipcore
          lai
          virtio_blk_serv
          block_partition
          kernel_schema
          machine_schema
          ae_usergroup
          agent_schema
)

target_include_directories(aekernel PUBLIC include)
