if (${TOS_ARCH} MATCHES "x86_64")
    add_lidlc(kernel_schema schema.lidl)
    add_lidl_dependency(kernel_schema filesystem_schema)
    add_lidl_dependency(kernel_schema caplets_schema)
    add_lidl_dependency(kernel_schema caplets_timeout_schema)

    target_sources(
      aekernel
      PRIVATE x86_64.cpp
              acpi.cpp
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
    include(CrossCmake)
    build_other_target(aekernel ${CMAKE_BINARY_DIR}/../cmake-build-barex64-user sample_group)

    include(FileEmbed)
    embed_file(aekernel group1 ${CMAKE_BINARY_DIR}/../cmake-build-barex64-user/bin/sample_group.stripped)
    return()
endif()

include(AddIso)

add_i386_loader(aekernel-loader ${CMAKE_BINARY_DIR}/../cmake-build-barex64 aekernel)
add_iso(aekernel-iso aekernel-loader)
