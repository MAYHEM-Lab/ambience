#include <block_memory_generated.hpp>
#include <tos/virtio/block_device.hpp>

namespace {
struct impl : public tos::ae::services::async_block_memory {
    tos::Task<int32_t> get_block_size() override {
        co_return m_blk_dev->sector_size_bytes();
    }
    tos::Task<int32_t> get_block_count() override {
        co_return m_blk_dev->number_of_sectors();
    }

    tos::virtio::block_device* m_blk_dev;
};
} // namespace

extern "C"
tos::ae::services::async_block_memory* init() {
    return new impl;
}