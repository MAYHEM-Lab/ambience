#include <arch/flash.hpp>
#include <algorithm>

namespace tos::cc32xx {
namespace {
void flash_isr() {
    tos::cc32xx::flash::get(0)->isr();
}
} // namespace
flash::flash()
    : tracked_driver(0) {
    FlashIntRegister(&flash_isr);
    FlashIntEnable(FLASH_INT_PROGRAM | FLASH_INT_ERASE_ERR | FLASH_INT_PROGRAM_ERR);
}

void flash::isr() {
    auto int_status = FlashIntStatus(true);
    if (int_status & FLASH_INT_PROGRAM) {
        FlashIntClear(FLASH_INT_PROGRAM);
        m_error = false;
    }
    if (int_status & FLASH_INT_ERASE_ERR) {
        FlashIntClear(FLASH_INT_ERASE_ERR);
        m_error = true;
    }
    if (int_status & FLASH_INT_PROGRAM_ERR) {
        FlashIntClear(FLASH_INT_PROGRAM_ERR);
        m_error = true;
    }
    m_wait.up_isr();
}

expected<void, flash_errors> flash::erase(sector_id_t sector_id) {
    auto addr = 0x01000000 + sector_id * sector_size_bytes();
    FlashEraseNonBlocking(addr);
    m_wait.down();
    if (m_error) {
        return unexpected(flash_errors::unknown);
    }
    return {};
}

expected<void, flash_errors>
flash::write(sector_id_t sector_id, span<const uint8_t> data, uint16_t offset) {
    auto addr = 0x01000000 + sector_id * sector_size_bytes() + offset;
    FlashProgramNonBlocking(
        const_cast<uint32_t*>(reinterpret_cast<const uint32_t*>(data.data())),
        addr,
        data.size());
    m_wait.down();
    return {};
}

expected<span<uint8_t>, flash_errors>
flash::read(sector_id_t sector_id, span<uint8_t> data, uint16_t offset) {
    auto addr = 0x01000000 + sector_id * sector_size_bytes() + offset;
    auto ptr = reinterpret_cast<const uint8_t*>(addr);
    std::copy_n(ptr, data.size(), data.data());
    return data;
}
} // namespace tos::cc32xx
