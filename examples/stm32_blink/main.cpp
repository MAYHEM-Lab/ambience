//
// Created by fatih on 10/25/18.
//

#include <tos/ft.hpp>

#include <libopencm3/stm32/gpio.h>
#include <tos/semaphore.hpp>

#include <drivers/arch/stm32/drivers.hpp>

tos::semaphore set{1}, clear{0};

void blink_task(void*)
{
	using namespace tos::tos_literals;

	tos::stm32::gpio g{ tos::stm32::ports[2] };
	g.set_pin_mode(45_pin, tos::pin_mode::out);

    while (true)
    {
        set.down();
        gpio_set(GPIOC, GPIO13);
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
        gpio_clear(GPIOC, GPIO13);
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