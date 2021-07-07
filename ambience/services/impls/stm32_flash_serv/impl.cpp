#include <arch/flash.hpp>
#include <block_memory_generated.hpp>

namespace {
using namespace tos::stm32;

struct impl final : tos::ae::services::block_memory::sync_server {
    int32_t get_block_size() override {
        return m_flash.sector_size_bytes();
    }

    int32_t get_block_count() override {
        return m_flash.number_of_sectors();
    }

    tos::span<uint8_t> read(const int32_t& block,
                            const int32_t& offset,
                            const int32_t& len,
                            ::lidl::message_builder& response_builder) override {
        auto start = response_builder.allocate(len, 1);
        auto res = tos::span<uint8_t>(start, len);
        m_flash.read(block, res, offset);
        return res;
    }

    bool
    write(const int32_t& block, const int32_t& offset, tos::span<uint8_t> data) override {
        m_flash.write(block, data, offset);
        return true;
    }

    bool buffered_write(const int32_t& block,
                        const int32_t& offset,
                        tos::span<uint8_t> data) override {
        // TODO: Buffer this write
        return write(block, offset, data);
    }

    bool erase(const int32_t& block) override {
        m_flash.erase(block);
        return true;
    }

    flash m_flash;
};
} // namespace

tos::ae::services::block_memory::sync_server* init_stm32_flash_serv() {
    return new impl;
}