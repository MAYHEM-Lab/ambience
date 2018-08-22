//
// Created by Mehmet Fatih BAKIR on 15/04/2018.
//

#include <drivers/arch/avr/usart.hpp>
#include <ft/include/tos/ft.hpp>
#include <tos/print.hpp>
#include <drivers/common/gpio.hpp>
#include <tos/devices.hpp>
#include <drivers/arch/avr/timer.hpp>
#include <tos/waitable.hpp>
#include <drivers/common/dht22.hpp>
#include <util/delay.h>
#include <tos/event.hpp>
#include <drivers/common/alarm.hpp>

void tick_task(void*)
{
    using namespace tos::tos_literals;

    auto g = tos::open(tos::devs::gpio);
    g->set_pin_mode(8_pin, tos::pin_mode::out);
    g->write(8_pin, true);

    auto tmr = open(tos::devs::timer<1>);
    auto alarm = open(tos::devs::alarm, *tmr);

    while (true)
    {
        alarm.sleep_for({ 5000 });
        g->write(8_pin, false);
        alarm.sleep_for({ 20 });
        g->write(8_pin, true);
    }
}

void tos_main()
{
    tos::launch(tick_task);
}