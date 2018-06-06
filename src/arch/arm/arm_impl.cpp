//
// Created by Mehmet Fatih BAKIR on 01/06/2018.
//

#include <stdbool.h>
#include <stdint.h>
#include <tos/ft.hpp>
#include <nrf52.h>
#include <nrf.h>

extern "C"
{
alignas(8) char stack[512*2];
int stack_index = 0;
void* tos_stack_alloc(size_t)
{
    return stack+512*stack_index++;
}

void tos_stack_free(void*)
{
}
}

void tos_main();

int main()
{
   // tos::enable_interrupts();

    NRF_CLOCK->LFCLKSRC = (uint32_t)((CLOCK_LFCLKSRC_SRC_Xtal << CLOCK_LFCLKSRC_SRC_Pos) & CLOCK_LFCLKSRC_SRC_Msk);
    NRF_CLOCK->TASKS_LFCLKSTART = 1UL;

    // RTC1 could be enabled by bootloader. Disable it
    NVIC_DisableIRQ(RTC1_IRQn);
    NRF_RTC1->EVTENCLR    = RTC_EVTEN_COMPARE0_Msk;
    NRF_RTC1->INTENCLR    = RTC_INTENSET_COMPARE0_Msk;
    NRF_RTC1->TASKS_STOP  = 1;
    NRF_RTC1->TASKS_CLEAR = 1;

    tos_main();

    while (true)
    {
        auto res = tos::schedule();
        if (res == tos::exit_reason::restart);// reboot();
        if (res == tos::exit_reason::power_down);// power_down(SLEEP_MODE_PWR_DOWN);
        if (res == tos::exit_reason::idle);// power_down(SLEEP_MODE_IDLE);
    }
}