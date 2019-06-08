//
// Created by fatih on 6/7/19.
//

#include <arch/drivers.hpp>
#include <tos/ft.hpp>
#include <libopencm3/stm32/exti.h>

using namespace tos::stm32;
using namespace tos::tos_literals;

void wifi_task()
{
    gpio_mode_setup(GPIOC, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO10 | GPIO11 | GPIO12);
    gpio_set_af(GPIOC, GPIO_AF6, GPIO10 | GPIO11 | GPIO12);

    auto reset_pin = 72_pin;
    auto cs_pin = 64_pin;
    auto dr_pin = 65_pin;
    auto wakeup_pin = 29_pin;
    auto boot0_pin = 28_pin;

    spi s(detail::spis[2]);
}

void tos_main()
{

}