#include <block_memory_generated.hpp>
#include <stdio.h>
#include <tos/detail/poll.hpp>
#include <tos/detail/tos_bind.hpp>
#include <tos/virtio/block_device.hpp>

namespace {
struct impl final : public tos::ae::services::block_memory::sync_server {
    explicit impl(tos::virtio::block_device* dev)
        : m_blk_dev(dev) {
    }

    int32_t get_block_size() override {
        return m_blk_dev->sector_size_bytes();
    }

    int32_t get_block_count() override {
        return m_blk_dev->number_of_sectors();
    }

    tos::span<uint8_t> read(const int32_t& block,
                            const int32_t& offset,
                            const int32_t& len,
                            lidl::message_builder& response_builder) override {
        auto start = response_builder.allocate(len, 1);
        auto res = tos::span<uint8_t>(start, len);

        if (len != get_block_size()) {
            std::vector<uint8_t> buffer(get_block_size());
            m_blk_dev->read(block, buffer, 0);
            tos::safe_span_copy(res, tos::span<const uint8_t>(buffer).slice(offset, len));
            return res;
        }

        m_blk_dev->read(block, res, offset);
        return res;
    }

    bool erase(const int32_t& block) override {
        return true;
    }

    bool
    write(const int32_t& block, const int32_t& offset, tos::span<uint8_t> data) override {
        if (static_cast<int>(data.size()) != get_block_size()) {
            std::vector<uint8_t> buffer(get_block_size());
            m_blk_dev->read(block, buffer, 0);
            tos::safe_span_copy(tos::span(buffer).slice(offset, data.size()),
                                tos::span<const uint8_t>(data));
            return write(block, 0, buffer);
        }

        if (!m_blk_dev->write(block, data, offset)) {
            return false;
        }

        return true;
    }

    bool buffered_write(const int32_t& block,
                        const int32_t& offset,
                        tos::span<uint8_t> data) override {
        if (static_cast<int>(data.size()) != get_block_size()) {
            return false;
        }

        m_write_sem.down();
        auto copied_data = std::vector<uint8_t>(data.begin(), data.end());
        tos::coro::make_detached(m_blk_dev->async_write(block, copied_data, 0),
                                 [&, copied_data = std::move(copied_data)] {
                                     m_write_sem.up_isr();
                                 });

        return true;
    }

    tos::semaphore m_write_sem{128};
    tos::virtio::block_device* m_blk_dev;
};
} // namespace

tos::ae::services::block_memory::sync_server*
init_virtio_blk(tos::virtio::block_device* dev) {
    return new impl(dev);
}