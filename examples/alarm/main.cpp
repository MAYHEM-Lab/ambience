//
// Created by Mehmet Fatih BAKIR on 11/05/2018.
//

#include <drivers/arch/avr/usart.hpp>
#include <ft/include/tos/ft.hpp>
#include <tos/print.hpp>
#include <tos/arch.hpp>
#include <drivers/common/gpio.hpp>
#include <tos/devices.hpp>
#include <drivers/arch/avr/timer.hpp>
#include <tos/waitable.hpp>
#include <drivers/common/alarm.hpp>

void tick_task()
{
    using namespace tos::tos_literals;
    auto usart = open(tos::devs::usart<0>, 19200_baud_rate);
    usart->options(
            tos::avr::usart_modes::async,
            tos::usart_parity::disabled,
            tos::usart_stop_bit::one);
    usart->enable();

    auto tmr = open(tos::devs::timer<1>);
    auto alarm = open(tos::devs::alarm, *tmr);

    tos::println(*usart, "Hello!");
    while (true)
    {
        tos::println(*usart, "yo");
        alarm.sleep_for({ 1000 });
    }
}

void tos_main()
{
    tos::launch(tick_task);
}