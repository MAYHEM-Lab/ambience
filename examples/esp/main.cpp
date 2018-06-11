//
// Created by fatih on 4/26/18.
//

extern "C"
{
#include "ets_sys.h"
#include "osapi.h"
#include "gpio.h"
}

#include <drivers/arch/lx106/usart.hpp>
#include <arch/lx106/tos_arch.hpp>
#include <tos/devices.hpp>
#include <tos/ft.hpp>

static const int pin = 2;

void other()
{
    int x = 5;
    while (true)
    {
        ++x;
        //gpio_output_set(0, (1 << pin), 0, 0);
        os_printf("Other: On, %p, %d\n", &x, x);
        tos::this_thread::yield();
        //gpio_output_set((1 << pin), 0, 0, 0);
        os_printf("Other: Off\n");
        tos::this_thread::yield();
    }
}

void task()
{
    using namespace tos::tos_literals;

    auto usart = open(tos::devs::usart<0>, 19200_baud_rate);
    usart->options(
            tos::usart_parity::disabled,
            tos::usart_stop_bit::one);
    usart->enable();

    //os_printf("hai");

    //gpio_output_set(0, 0, (1 << pin), 0);

    int x = 15;
    int y = 17;
    os_printf("base sp: %p, %p, %d\n", read_sp(), &x, x);
    while (true)
    {
        //++x;
        //gpio_output_set(0, (1 << pin), 0, 0);
        os_printf("Task: On\n");
        os_printf("base sp: %p, %p, %d\n", read_sp(), &x, x);
        os_printf("base sp: %p, %p, %d\n", read_sp(), &y, y);

        tos::this_thread::yield();
        //gpio_output_set((1 << pin), 0, 0, 0);
        os_printf("Task: Off\n");
        tos::this_thread::yield();
    }
}

void tos_init()
{
    gpio_init();

    int x = 15;
    os_printf("base sp: %p, %p, %d\n", read_sp(), &x, x);
    tos::launch(task);
    tos::launch(other);
}