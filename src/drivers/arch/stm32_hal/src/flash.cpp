#include "tos/expected.hpp"
#include "tos/mutex.hpp"
#include "tos/thread.hpp"
#include <arch/detail/flash.hpp>
#include <arch/flash.hpp>
#include <cstdint>
#include <cstring>
#include <stm32_hal/flash.hpp>
#include <stm32_hal/flash_ex.hpp>

extern "C" {
void FLASH_IRQHandler() {
    HAL_FLASH_IRQHandler();
}

void HAL_FLASH_EndOfOperationCallback(uint32_t page) {
    tos::stm32::flash::get(0)->isr(page);
}

void HAL_FLASH_OperationErrorCallback(uint32_t page) {
    tos::stm32::flash::get(0)->isr_fail(page);
}
}

namespace tos::stm32 {
flash::flash()
    : tracked_driver(0)
    , m_last_op_fail() {
    HAL_NVIC_SetPriority(FLASH_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(FLASH_IRQn);
}

tos::expected<void, flash_errors> flash::erase(sector_id_t sector_id) {
    auto unlock_status = HAL_FLASH_Unlock();

    clear_flash_flags();

    if (unlock_status != HAL_OK) {
        return unexpected(flash_errors::unlock_failed);
    }

    const uint32_t addr = info.base_addr + (info.page_size * sector_id);

    if ((addr % sizeof(flash_align_t)) != 0) {
        return unexpected(flash_errors::bad_align);
    }

    FLASH_EraseInitTypeDef erase_info{};
#if defined(STM32L4)
    erase_info.TypeErase = FLASH_TYPEERASE_PAGES;
    erase_info.Banks = sector_id < number_of_sectors() / 2 ? FLASH_BANK_1 : FLASH_BANK_2;
    erase_info.Page = sector_id;
    erase_info.NbPages = 1;
#elif defined(STM32L0) || defined(STM32F1)
    erase_info.TypeErase = FLASH_TYPEERASE_PAGES;
    erase_info.PageAddress = addr;
    erase_info.NbPages = 1;
#elif defined(STM32F7)
    erase_info.TypeErase = FLASH_TYPEERASE_SECTORS;
    erase_info.Sector = sector_id;
    erase_info.NbSectors = 1;
#endif

    auto erase_status = HAL_FLASHEx_Erase_IT(&erase_info);
    if (erase_status != HAL_OK) {
        return unexpected(flash_errors::erase_failed);
    }

    tos::kern::busy();
    m_done.down();
    tos::kern::unbusy();

    if (m_last_op_fail) {
        return unexpected(flash_errors::operation_failed);
    }

    return {};
}

expected<void, flash_errors>
flash::write(sector_id_t sector_id, span<const uint8_t> data, size_t offset) {
    if ((data.size() % sizeof(flash_align_t)) != 0) {
        return unexpected(flash_errors::bad_size);
    }

    auto unlock_status = HAL_FLASH_Unlock();

    clear_flash_flags();

    if (unlock_status != HAL_OK) {
        return unexpected(flash_errors::unlock_failed);
    }

    uintptr_t addr = info.base_addr + (info.page_size * sector_id) + offset;

    if ((addr % sizeof(flash_align_t)) != 0) {
        return unexpected(flash_errors::bad_align);
    }

    while (!data.empty()) {
        flash_align_t temp;
        std::memcpy(&temp, data.data(), sizeof temp);
        auto write_res = write_one_async(addr, temp);

        if (write_res != HAL_OK) {
            return unexpected(flash_errors::operation_failed);
        }

        tos::kern::busy();
        m_done.down();
        tos::kern::unbusy();

        if (m_last_op_fail) {
            return unexpected(flash_errors::operation_failed);
        }

        data = data.slice(sizeof temp);
        addr += sizeof temp;
    }

    return {};
}

expected<void, flash_errors>
flash::read(flash::sector_id_t sector_id, span<uint8_t> data, size_t offset) {
    if (data.size() % sizeof(flash_align_t) != 0) {
        return unexpected(flash_errors::bad_size);
    }

    uintptr_t addr = info.base_addr + (info.page_size * sector_id) + offset;
    if ((addr % sizeof(flash_align_t)) != 0) {
        return unexpected(flash_errors::bad_align);
    }

    while (!data.empty()) {
        flash_align_t temp;
        auto ptr = reinterpret_cast<const flash_align_t*>(addr);
        temp = *ptr;
        std::memcpy(data.data(), &temp, sizeof temp);
        data = data.slice(sizeof temp);
        addr += sizeof temp;
    }

    return expected<void, flash_errors>();
}

void flash::isr(uint32_t) {
    m_last_op_fail = false;
    m_done.up_isr();
}

void flash::isr_fail(uint32_t) {
    m_last_op_fail = true;
    m_done.up_isr();
}
} // namespace tos::stm32