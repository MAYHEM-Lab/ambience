if (${TOS_ARCH} MATCHES "x86_64")
    target_sources(aekernel PRIVATE x86_64.cpp)
    target_link_libraries(aekernel PRIVATE uart_16550 tos_elf vga_text)

    include(FileEmbed)

    add_dependencies(aekernel sample_group)
    embed_file(aekernel group1 $<TARGET_FILE:sample_group>.stripped)
elseif (${TOS_PLATFORM} MATCHES "raspi")
    target_sources(aekernel PRIVATE raspi3.cpp)
    target_link_libraries(aekernel PRIVATE arch_drivers bcm2837_clock tos_elf)

    include(FileEmbed)

    add_dependencies(aekernel sample_group)
    embed_file(aekernel group1 $<TARGET_FILE:sample_group>.stripped)
elseif (${TOS_PLATFORM} MATCHES "hosted")
    target_sources(aekernel PRIVATE hosted.cpp)
    target_link_libraries(aekernel PRIVATE arch_drivers)
elseif (${TOS_PLATFORM} MATCHES "stm32")
    target_sources(aekernel PRIVATE stm32.cpp)
    target_link_libraries(aekernel PRIVATE tos_board)
endif()