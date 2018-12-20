//
// Created by Mehmet Fatih BAKIR on 01/06/2018.
//

#include <stdbool.h>
#include <stdint.h>
#include <tos/ft.hpp>
#include <nrf52.h>
#include <nrf.h>
#include <tos/interrupt.hpp>
#include <stdlib.h>

extern "C"
{
    void NORETURN tos_force_reset()
    {
        NVIC_SystemReset();
    }

alignas(8) char stack[512*2];
int stack_index = 0;
void* tos_stack_alloc(size_t sz)
{
    return malloc(sz);
    return stack+512*stack_index++;
}

void tos_stack_free(void* ptr)
{
    return free(ptr);
}
}

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
        if (res == tos::exit_reason::power_down) __WFI();// power_down(SLEEP_MODE_PWR_DOWN);
        if (res == tos::exit_reason::idle) __WFE();// power_down(SLEEP_MODE_IDLE);
    }
}