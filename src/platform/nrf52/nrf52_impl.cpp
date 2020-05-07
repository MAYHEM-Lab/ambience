//
// Created by Mehmet Fatih BAKIR on 01/06/2018.
//

#include "tos/tcb.hpp"

#include <app_util_platform.h>
#include <nrf.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <tos/ft.hpp>
#include <tos/interrupt.hpp>
#include <nrf_pwr_mgmt.h>

#ifdef SOFTDEVICE_PRESENT
#include "nrf_sdh.h"
#include "nrf_soc.h"
#endif // SOFTDEVICE_PRESENT

void tos_main();

int main() {
    tos::kern::enable_interrupts();

    nrf_pwr_mgmt_init();
    tos_main();

    while (true) {
        auto res = tos::global::sched.schedule();
        if (res == tos::exit_reason::restart)
            NVIC_SystemReset(); // reboot();
        if (res == tos::exit_reason::power_down || res == tos::exit_reason::idle) {
            if (nrf_sdh_is_enabled()) {
                ret_code_t ret_code = sd_app_evt_wait();
                ASSERT((ret_code == NRF_SUCCESS) ||
                       (ret_code == NRF_ERROR_SOFTDEVICE_NOT_ENABLED));
                UNUSED_VARIABLE(ret_code);
            } else {
                __WFE();
                __SEV();
                __WFE();
            }
        }
    }
}