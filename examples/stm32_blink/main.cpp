//
// Created by fatih on 10/25/18.
//

#include <tos/ft.hpp>

#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>
#include <tos/semaphore.hpp>

constexpr auto RCC_LED1 = RCC_GPIOC;
constexpr auto PORT_LED1 = GPIOC;
constexpr auto PIN_LED1 = GPIO13;

tos::semaphore set{1}, clear{0};

void blink_task(void*)
{
    rcc_periph_clock_enable(RCC_LED1);
    gpio_set_mode(PORT_LED1, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, PIN_LED1);
    gpio_set(PORT_LED1, PIN_LED1);

    while (true)
    {
        set.down();
        gpio_set(PORT_LED1, PIN_LED1);
        for (int i = 0; i < 2'000'000; i++) {
            __asm__("nop");
        }
        clear.up();
    }
}

void off_task(void*)
{
    while (true)
    {
        clear.down();
        gpio_clear(PORT_LED1, PIN_LED1);
        for (int i = 0; i < 2'000'000; i++) {
            __asm__("nop");
        }
        set.up();
    }
}

void tos_main()
{
    tos::launch(blink_task);
    tos::launch(off_task);
}