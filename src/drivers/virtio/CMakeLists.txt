if (NOT ${TOS_PLATFORM} MATCHES "x86_64")
    return()
endif()

add_library(tos_virtio queue.cpp device.cpp include/tos/virtio/mmio.hpp include/tos/virtio/transport.hpp include/tos/virtio/x86_pci_transport.hpp x86_pci_transport.cpp)
target_include_directories(tos_virtio PUBLIC include)
target_link_libraries(tos_virtio PUBLIC tos_drivers arch_x86_64)

add_library(tos_virtio_block_dev block_device.cpp)
target_link_libraries(tos_virtio_block_dev PUBLIC tos_virtio)

add_library(tos_virtio_network_dev network_device.cpp)
target_link_libraries(tos_virtio_network_dev PUBLIC tos_virtio tos_inet)
#target_compile_options(tos_virtio_network_dev PRIVATE -mstackrealign)

add_library(tos_virtio_netif virtio_netif.cpp)
target_link_libraries(tos_virtio_netif PUBLIC tos_virtio_network_dev lwipcore)