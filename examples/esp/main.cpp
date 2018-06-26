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
#include <drivers/arch/lx106/timer.hpp>
#include <drivers/common/alarm.hpp>
#include <ft/include/tos/semaphore.hpp>

static const int pin = 2;

tos::semaphore sem{0};

void other()
{
    int x = 5;
    while (true)
    {
        ++x;
        os_printf("Other: On, %p, %d\n", &x, x);
        sem.down();

        os_printf("Other: Off\n");
        sem.down();
    }
}

void task()
{
    using namespace tos::tos_literals;

    auto usart = open(tos::devs::usart<0>, 19200_baud_rate);
    usart->enable();
    os_printf("\n\n\n\n\n");

    auto tmr = open(tos::devs::timer<0>);
    auto alarm = tos::open(tos::devs::alarm, tmr);

    int x = 15;
    int y = 17;

    while (true)
    {
        ++x;
        os_printf("Task: On\n");
        os_printf("base sp: %p, %p, %d\n", read_sp(), &x, x);
        os_printf("base sp: %p, %p, %d\n", read_sp(), &y, y);
        sem.up();
        alarm.sleep_for({ 500 });

        sem.up();
        os_printf("Task: Off\n");
        alarm.sleep_for({ 500 });
    }
}

void tos_init()
{
    tos::launch(task);
    tos::launch(other);
}