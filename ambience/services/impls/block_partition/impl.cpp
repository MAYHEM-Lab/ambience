#include <block_memory_generated.hpp>

namespace {
struct impl : tos::ae::services::block_memory::sync_server {
    impl(tos::ae::services::block_memory::sync_server* base,
         int base_block,
         int block_count)
        : m_base{base}
        , m_base_block{base_block}
        , m_block_count{block_count} {
    }

    int32_t get_block_size() override {
        return m_base->get_block_size();
    }

    int32_t get_block_count() override {
        return m_block_count;
    }

    tos::span<uint8_t> read(const int32_t& block,
                            const int32_t& offset,
                            const int32_t& len,
                            lidl::message_builder& response_builder) override {
        return m_base->read(block + m_base_block, offset, len, response_builder);
    }

    bool erase(const int32_t& block) override {
        return m_base->erase(block + m_base_block);
    }

    bool
    write(const int32_t& block, const int32_t& offset, tos::span<uint8_t> data) override {
        return m_base->write(block + m_base_block, offset, data);
    }

    bool buffered_write(const int32_t& block,
                        const int32_t& offset,
                        tos::span<uint8_t> data) override {
        return m_base->buffered_write(block + m_base_block, offset, data);
    }

    tos::ae::services::block_memory::sync_server* m_base;
    int m_base_block;
    int m_block_count;
};
} // namespace


tos::ae::services::block_memory::sync_server* init_block_partiton(
    tos::ae::services::block_memory::sync_server* dev, int base_block, int block_count) {
    return new impl(dev, base_block, block_count);
}
