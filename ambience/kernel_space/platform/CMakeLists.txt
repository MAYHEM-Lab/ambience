if (${TOS_ARCH} MATCHES "x86_64")
    target_sources(aekernel PRIVATE x86_64.cpp)
    target_link_libraries(aekernel PRIVATE uart_16550)
endif()