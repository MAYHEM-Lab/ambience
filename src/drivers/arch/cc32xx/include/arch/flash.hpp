#pragma once

#include <ti/devices/cc32xx/driverlib/flash.h>
#include <tos/self_pointing.hpp>
#include <common/driver_base.hpp>
#include <tos/utility.hpp>
#include <tos/span.hpp>
#include <cstdint>
#include <tos/span.hpp>
#include <tos/semaphore.hpp>
#include <tos/expected.hpp>

namespace tos::cc32xx {
enum class flash_errors
{
    bad_address,
    bad_alignment,
    bad_protection,
    unknown
};
class flash
    : public self_pointing<flash>
        , public tracked_driver<flash, 1>
        , public non_copy_movable {
public:
    using sector_id_t = uint16_t;

    flash();

    expected<void, flash_errors> erase(sector_id_t sector_id);
    expected<void, flash_errors>
    write(sector_id_t sector_id, span<const uint8_t> data, uint16_t offset);
    expected<span<uint8_t>, flash_errors>
    read(sector_id_t sector_id, span<uint8_t> data, uint16_t offset);

    size_t sector_size_bytes() const {
        return 2048;
    }

    size_t sector_count() const {
        return 512;
    }

    void isr();

private:
    bool m_error = false;
    tos::semaphore m_wait{0};
};
} // namespace tos::cc32xx
