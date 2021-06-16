#include <arch/flash.hpp>
#include <block_memory_generated.hpp>

namespace {
using namespace tos::stm32;

struct impl : tos::ae::services::block_memory::sync_server {
    int32_t get_block_size() override {
        return m_flash.sector_size_bytes();
    }
    int32_t get_block_count() override {
        return m_flash.number_of_sectors();
    }

    flash m_flash;
};
} // namespace

std::unique_ptr<tos::ae::services::block_memory::sync_server> init() {
    return std::make_unique<impl>();
}