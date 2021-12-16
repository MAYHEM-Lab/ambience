target_sources(aekernel PRIVATE
    ethernet.cpp
    uspi_host.cpp
    page_allocator.cpp
    platform_support.cpp
)

target_link_libraries(aekernel PUBLIC arch_drivers bcm2837_clock lwipcore ae_usergroup)
target_link_libraries(aekernel PUBLIC uspi ephemeral_block)
target_include_directories(aekernel PUBLIC include)