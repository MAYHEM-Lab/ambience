target_include_directories(aekernel PUBLIC include)
target_link_libraries(aekernel PUBLIC tos_board)
target_sources(
        aekernel
        PRIVATE
        platform_support.cpp
        udp_transport.cpp
        udp_host.cpp
)
