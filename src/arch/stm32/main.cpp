//
// Created by fatih on 10/25/18.
//

#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>
#include <tos/compiler.hpp>
#include <tos/scheduler.hpp>
#include <tos/ft.hpp>
#include <tos_arch.hpp>
#include <libopencmsis/core_cm3.h>

extern "C"
{
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
    rcc_clock_setup_in_hse_8mhz_out_72mhz();

    /* Enable GPIOA clock (for LED GPIOs). */
    rcc_periph_clock_enable(RCC_GPIOA);

    /* Enable clocks for GPIO port B (for GPIO_USART1_TX) and USART1. */
    rcc_periph_clock_enable(RCC_GPIOB);
    rcc_periph_clock_enable(RCC_AFIO);
    rcc_periph_clock_enable(RCC_USART1);

    tos_main();

    while (true)
    {
        auto res = tos::kern::schedule();
        //if (res == tos::exit_reason::restart) NVIC_SystemReset();// reboot();
        if (res == tos::exit_reason::power_down) __WFI();// power_down(SLEEP_MODE_PWR_DOWN);
        if (res == tos::exit_reason::idle) __asm__("wfe");// power_down(SLEEP_MODE_IDLE);
    }
}