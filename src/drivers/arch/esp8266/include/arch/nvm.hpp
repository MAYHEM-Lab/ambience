//
// Created by fatih on 2/24/19.
//

#pragma once

#include <tos/expected.hpp>
#include <tos/interrupt.hpp>
#include <tos/span.hpp>

extern "C" {
#include <spi_flash.h>
}

namespace tos {
namespace esp82 {
enum class flash_errors
{
    err = 1,
    timeout = 2,
    misaligned_data,
    misaligned_size
};

class spi_flash {
public:
    using sector_id_t = uint16_t;

    size_t sector_size_bytes() const { return SPI_FLASH_SEC_SIZE; }

    expected<void, flash_errors> erase(sector_id_t sector) {
        tos::int_guard ig;
        auto res = spi_flash_erase_sector(sector);
        if (res == SPI_FLASH_RESULT_OK)
            return {};
        else {
            return unexpected(flash_errors(res));
        }
    }

    expected<void, flash_errors>
    write(sector_id_t sector, tos::span<const uint8_t> data, size_t offset = 0) {
        tos::int_guard ig;

        if (reinterpret_cast<uintptr_t>(data.data()) % 4 != 0) {
            return unexpected(flash_errors::misaligned_data);
        }
        if (data.size() % 4 != 0) {
            return unexpected(flash_errors::misaligned_size);
        }

        auto res = spi_flash_write(
            (uint32_t)sector * sector_size_bytes() + offset, (uint32*)data.data(), data.size());
        if (res == SPI_FLASH_RESULT_OK) {
            return {};
        } else {
            return unexpected(flash_errors(res));
        }
    }

    expected<void, flash_errors>
    read(sector_id_t sector, tos::span<uint8_t> buf, size_t offset = 0) {
        tos::int_guard ig;

        if (reinterpret_cast<uintptr_t>(buf.data()) % 4 != 0) {
            return unexpected(flash_errors::misaligned_data);
        }
        if (buf.size() % 4 != 0) {
            return unexpected(flash_errors::misaligned_size);
        }

        auto res = spi_flash_read(
            (uint32_t)sector * sector_size_bytes() + offset, (uint32*)buf.data(), buf.size());
        if (res == SPI_FLASH_RESULT_OK) {
            return {};
        } else {
            return unexpected(flash_errors(res));
        }
    }

private:
};
} // namespace esp82
} // namespace tos
