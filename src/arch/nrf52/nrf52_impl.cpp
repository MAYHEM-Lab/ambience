//
// Created by Mehmet Fatih BAKIR on 01/06/2018.
//

#include <stdbool.h>
#include <stdint.h>
#include <tos/ft.hpp>
#include <nrf.h>
#include <tos/interrupt.hpp>
#include <stdlib.h>
#include <app_util_platform.h>

#ifdef SOFTDEVICE_PRESENT
#include "nrf_soc.h"
#include "nrf_sdh.h"
#endif // SOFTDEVICE_PRESENT

void tos_main();

int main()
{
    tos::kern::enable_interrupts();

    /* Start 16 MHz crystal oscillator */
    NRF_CLOCK->EVENTS_HFCLKSTARTED = 0;
    NRF_CLOCK->TASKS_HFCLKSTART    = 1;

    /* Wait for the external oscillator to start up */
    while (NRF_CLOCK->EVENTS_HFCLKSTARTED == 0)
    {
        // Do nothing.
    }

    /* Start low frequency crystal oscillator for app_timer(used by bsp)*/
    NRF_CLOCK->LFCLKSRC            = (CLOCK_LFCLKSRC_SRC_Xtal << CLOCK_LFCLKSRC_SRC_Pos);
    NRF_CLOCK->EVENTS_LFCLKSTARTED = 0;
    NRF_CLOCK->TASKS_LFCLKSTART    = 1;

    while (NRF_CLOCK->EVENTS_LFCLKSTARTED == 0)
    {
        // Do nothing.
    }

    // RTC1 could be enabled by bootloader. Disable it
    NVIC_DisableIRQ(RTC1_IRQn);
    NRF_RTC1->EVTENCLR    = RTC_EVTEN_COMPARE0_Msk;
    NRF_RTC1->INTENCLR    = RTC_INTENSET_COMPARE0_Msk;
    NRF_RTC1->TASKS_STOP  = 1;
    NRF_RTC1->TASKS_CLEAR = 1;

    tos_main();

    while (true)
    {
        auto res = tos::kern::schedule();
        if (res == tos::exit_reason::restart) NVIC_SystemReset();// reboot();
        if (res == tos::exit_reason::power_down || res == tos::exit_reason::idle) {
            if (nrf_sdh_is_enabled())
            {
                ret_code_t ret_code = sd_app_evt_wait();
                ASSERT((ret_code == NRF_SUCCESS) || (ret_code == NRF_ERROR_SOFTDEVICE_NOT_ENABLED));
                UNUSED_VARIABLE(ret_code);
            } else{
                __WFE();
                __SEV();
                __WFE();
            }
        }
    }
}