if (${TOS_ARCH} MATCHES "x86_64")
    target_sources(aekernel PRIVATE
        x86_64.cpp
        elf.hpp
    )
    target_link_libraries(aekernel PRIVATE
        uart_16550
        tos_elf
        vga_text
        tos_virtio
        tos_virtio_block_dev
        tos_virtio_network_dev
        lwipcore
        lai
    )

    include(FileEmbed)

    add_dependencies(aekernel sample_group)
    embed_file(aekernel group1 $<TARGET_FILE:sample_group>.stripped)
elseif (${TOS_PLATFORM} MATCHES "raspi")
    target_sources(aekernel PRIVATE
        raspi3.cpp
        elf.hpp
    )
    target_link_libraries(aekernel PRIVATE arch_drivers bcm2837_clock tos_elf lwipcore)

    include(FileEmbed)

    add_dependencies(aekernel sample_group)
    embed_file(aekernel group1 $<TARGET_FILE:sample_group>.stripped)
elseif (${TOS_PLATFORM} MATCHES "hosted")
    target_sources(aekernel PRIVATE hosted.cpp)
    target_link_libraries(aekernel PRIVATE arch_drivers)
elseif (${TOS_PLATFORM} MATCHES "stm32")
    target_sources(aekernel PRIVATE stm32.cpp)
    target_link_libraries(aekernel PRIVATE tos_board)
endif ()