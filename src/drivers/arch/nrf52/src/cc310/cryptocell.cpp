#include "tos/expected.hpp"

#include <arch/cc310/cryptocell.hpp>
#include <nrf52840.h>
#include <sns_silib.h>

extern "C" void CRYPTOCELL_IRQHandler();
extern "C" void CryptoCell_Handler() {
    CRYPTOCELL_IRQHandler();
}

namespace tos::nrf52::cc310 {
cryptocell::~cryptocell() {
    SaSi_LibFini();
    NRF_CRYPTOCELL->ENABLE = 0;
    NVIC_DisableIRQ(CRYPTOCELL_IRQn);
}

expected<cryptocell, cc_errors> open_cryptocell() {
    if (NRF_CRYPTOCELL->ENABLE) {
        return unexpected(cc_errors::already_open);
    }
    NVIC_EnableIRQ(CRYPTOCELL_IRQn);
    NRF_CRYPTOCELL->ENABLE = 1;
    auto ret = SaSi_LibInit();
    if (ret != SA_SILIB_RET_OK) {
        return unexpected(cc_errors::unknown_error);
    }
    return {default_construct};
}
} // namespace tos::nrf52::cc310