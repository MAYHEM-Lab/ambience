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

tos::usart comm;

void tick_task()
{
    using namespace tos::tos_literals;
    auto usart = open(tos::devs::usart<0>, 19200_baud_rate);
    usart->options(
            tos::usart_modes::async,
            tos::usart_parity::disabled,
            tos::usart_stop_bit::one);
    usart->enable();

    auto g = tos::open(tos::devs::gpio);
    g->set_pin_mode(8_pin, tos::pin_mode_t::out);
    g->write(8_pin, true);

    auto tmr = open(tos::devs::timer<1>);
    tos::alarm<tos::remove_reference_t<decltype(*tmr)>> alarm{*tmr};

    while (true)
    {
        alarm.sleep_for({ 5000 });
        g->write(8_pin, false);
        alarm.sleep_for({ 20 });
        g->write(8_pin, true);
    }
}

int main()
{
    tos::enable_interrupts();

    tos::launch(tick_task);

    while(true)
    {
        tos::schedule();
    }
}