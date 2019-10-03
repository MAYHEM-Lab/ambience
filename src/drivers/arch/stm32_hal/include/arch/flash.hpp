#pragma once

#include "common/driver_base.hpp"
#include "detail/flash.hpp"
#include "tos/mutex.hpp"

#include <cstdint>
#include <tos/expected.hpp>
#include <tos/semaphore.hpp>
#include <tos/span.hpp>

namespace tos::stm32 {
enum class flash_errors
{
    unlock_failed,
    erase_failed,
    operation_failed,
    bad_size,
    bad_align
};

class flash
    : public tracked_driver<flash, 1>
    , private detail::flash_ll {
public:
    using sector_id_t = uint32_t;

    flash();

    /**
     * Returns the size of each flash sector/page in bytes.
     */
    size_t sector_size_bytes() const {
        return flash_ll::info.page_size;
    }

    /**
     * Number of sectors in the flash.
     */
    size_t number_of_sectors() const {
        return flash_ll::info.num_pages;
    }

    /**
     * Erases the contents of the sector specified by its id.
     */
    expected<void, flash_errors> erase(sector_id_t sector_id);

    expected<void, flash_errors>
    write(sector_id_t sector_id, span<const char> data, size_t offset);

    expected<void, flash_errors>
    read(sector_id_t sector_id, span<char> data, size_t offset);

    void isr(uint32_t page);
    void isr_fail(uint32_t page);

private:
    bool m_last_op_fail;
    semaphore m_done{0};
};
} // namespace tos::stm32
