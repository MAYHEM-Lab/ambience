if (NOT TARGET tos_virtio_block_dev)
    return()
endif()

add_service(virtio_blk_serv block_memory impl.cpp)
target_link_libraries(virtio_blk_serv PUBLIC tos_virtio_block_dev)
