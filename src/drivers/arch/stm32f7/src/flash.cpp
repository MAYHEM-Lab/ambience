#include "tos/expected.hpp"
#include "tos/mutex.hpp"
#include "tos/thread.hpp"

#include <arch/flash.hpp>
#include <cstdint>
#include <cstring>
#include <stm32_hal/flash.hpp>
#include <stm32_hal/flash_ex.hpp>
#include <stm32l4xx_hal_flash.h>

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

namespace {
constexpr auto page_size = 2048;
constexpr auto base_addr = 0x0800'0000;
} // namespace

namespace tos::stm32 {
flash::flash()
    : tracked_driver(0)
    , m_last_op_fail() {
    HAL_NVIC_SetPriority(FLASH_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(FLASH_IRQn);
}

tos::expected<void, flash_errors> flash::erase(sector_id_t sector_id) {
    auto unlock_status = HAL_FLASH_Unlock();

    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ALL_ERRORS);

    if (unlock_status != HAL_OK) {
        return unexpected(flash_errors::unlock_failed);
    }

    FLASH_EraseInitTypeDef erase_info{};
    erase_info.TypeErase = FLASH_TYPEERASE_PAGES;
    erase_info.Banks = FLASH_BANK_1;
    erase_info.Page = sector_id;
    erase_info.NbPages = 1;
    auto erase_status = HAL_FLASHEx_Erase_IT(&erase_info);
    if (erase_status != HAL_OK) {
        return unexpected(flash_errors::erase_failed);
    }

    m_done.down();

    if (m_last_op_fail) {
        return unexpected(flash_errors::operation_failed);
    }

    return {};
}

expected<void, flash_errors>
flash::write(sector_id_t sector_id, span<const char> data, size_t offset) {
    if ((data.size() % sizeof(uint64_t)) != 0) {
        return unexpected(flash_errors::bad_size);
    }

    auto unlock_status = HAL_FLASH_Unlock();

    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ALL_ERRORS);

    if (unlock_status != HAL_OK) {
        return unexpected(flash_errors::unlock_failed);
    }

    uint32_t addr = base_addr + (page_size * sector_id) + offset;

    if ((addr % sizeof(uint64_t)) != 0) {
        return unexpected(flash_errors::bad_align);
    }

    while (!data.empty()) {
        uint64_t temp;
        std::memcpy(&temp, data.data(), sizeof temp);
        auto write_res = HAL_FLASH_Program_IT(FLASH_TYPEPROGRAM_DOUBLEWORD, addr, temp);

        if (write_res != HAL_OK) {
            return unexpected(flash_errors::operation_failed);
        }

        m_done.down();

        if (m_last_op_fail) {
            return unexpected(flash_errors::operation_failed);
        }

        data = data.slice(sizeof temp);
        addr += sizeof temp;
    }

    return {};
}

expected<void, flash_errors>
flash::read(flash::sector_id_t sector_id, span<char> data, size_t offset) {


    if(data.size()% sizeof(uint64_t)!=0){
        return unexpected(flash_errors::bad_size);
    }

    uintptr_t addr = base_addr + (page_size * sector_id) + offset;
    if ((addr % sizeof(uint64_t)) != 0) {
        return unexpected(flash_errors::bad_align);
    }

    while (!data.empty()) {
        uint64_t temp;
        auto ptr = reinterpret_cast<const uint64_t *>(addr);
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