#pragma once

#include <cstddef>
#include <cstdint>
#include <tos/expected.hpp>
#include <tos/span.hpp>

namespace tos::nrf52 {
enum class flash_errors
{
    bad_address,
    bad_alignment,
    bad_protection,
    unknown
};
class flash {
public:
    using sector_id_t = uint16_t;
    flash();

    size_t sector_count() const;
    size_t sector_size_bytes() const;

    expected<void, flash_errors> erase(sector_id_t sector_id);
    expected<void, flash_errors>
    write(sector_id_t sector_id, span<const uint8_t> data, uint16_t offset);
    expected<span<uint8_t>, flash_errors>
    read(sector_id_t sector_id, span<uint8_t> data, uint16_t offset);

private:

    uintptr_t translate_address(sector_id_t sector, uint16_t offset) const;
};
} // namespace tos::nrf52