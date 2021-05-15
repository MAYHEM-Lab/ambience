target_sources(aekernel PRIVATE
    ethernet.cpp
    uspi_host.cpp
)
target_link_libraries(aekernel PRIVATE uspi)