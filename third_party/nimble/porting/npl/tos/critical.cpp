#include "tos/platform.hpp"
#include <nimble/nimble_npl.h>
#include <tos/interrupt.hpp>

extern "C" {
uint32_t ble_npl_hw_enter_critical(void) {
    tos::kern::disable_interrupts(__builtin_return_address(0));
    return 0;
}

void ble_npl_hw_exit_critical(uint32_t ctx) {
    tos::kern::enable_interrupts(__builtin_return_address(0));
}

bool ble_npl_hw_is_in_critical(void) {
    return tos::platform::interrupts_disabled();
}
}