#include <block_memory_generated.hpp>
#include <tos/virtio/block_device.hpp>

namespace {
struct impl final : public tos::ae::services::block_memory::async_server {
    explicit impl(tos::virtio::block_device* dev)
        : m_blk_dev(dev) {
    }

    tos::Task<int32_t> get_block_size() override {
        co_return m_blk_dev->sector_size_bytes();
    }

    tos::Task<int32_t> get_block_count() override {
        co_return m_blk_dev->number_of_sectors();
    }

    tos::Task<tos::span<uint8_t>> read(const int32_t& block,
                                       const int32_t& offset,
                                       lidl::message_builder& response_builder) override {
        auto& res = lidl::create_vector_sized<uint8_t>(response_builder, m_blk_dev->sector_size_bytes() - offset);
        co_await m_blk_dev->async_read(block, res, offset);
        co_return tos::span<uint8_t>(res);
    }

    tos::virtio::block_device* m_blk_dev;
};
} // namespace

tos::ae::services::block_memory::async_server*
init_virtio_blk(tos::virtio::block_device* dev) {
    return new impl(dev);
}