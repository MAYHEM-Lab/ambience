#include <arch/flash.hpp>
#include <nrfx_nvmc.h>

namespace tos::nrf52 {
flash::flash() {
}

size_t flash::sector_count() const {
    return nrfx_nvmc_flash_page_count_get();
}

size_t flash::sector_size_bytes() const {
    return nrfx_nvmc_flash_page_size_get();
}

expected<void, flash_errors> flash::erase(sector_id_t sector_id) {
    auto res = nrfx_nvmc_page_erase(translate_address(sector_id, 0));
    if (res == NRFX_SUCCESS) {
        return {};
    }
    if (res == NRFX_ERROR_INVALID_ADDR) {
        return unexpected(flash_errors::bad_address);
    }
    return unexpected(flash_errors::unknown);
}
} // namespace tos::nrf52