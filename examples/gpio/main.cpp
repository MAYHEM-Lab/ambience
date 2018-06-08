//
// Created by Mehmet Fatih BAKIR on 29/03/2018.
//

#include <ft/include/tos/ft.hpp>
#include <tos/semaphore.hpp>
#include <tos_arch.hpp>
#include <tos/print.hpp>
#include <tos/devices.hpp>

#include <drivers/arch/avr/usart.hpp>

#include <avr/interrupt.h>
#include <tos/intrusive_ptr.hpp>
#include <drivers/arch/avr/gpio.hpp>
#include <drivers/arch/avr/timer.hpp>
#include <drivers/common/alarm.hpp>

auto gp = open(tos::devs::gpio);

using namespace tos::tos_literals;
auto usart = open(tos::devs::usart<0>, 19200_baud_rate);

void hello_task()
{
    using namespace tos::tos_literals;
    gp->set_pin_mode(13_pin, tos::pin_mode::out);
    auto p = 13_pin;
    tos::println(*usart, "Pin: ", (int)p.pin);
    gp->write(13_pin, false);
    char buf;
    while (true) {

        usart->read({&buf ,1});
        if (buf == '1')
        {
            gp->write(13_pin, true);
            tos::println(*usart, "On");
        }
        else
        {
            gp->write(13_pin, false);
            tos::println(*usart, "Off");
        }
    }
}

void tick_task()
{
    using namespace tos::tos_literals;
    auto p = 8_pin;
    tos::println(*usart, "Pin: ", (int)p.pin);
    tos::println(*usart, "Port: ", (uintptr_t)&p.port->data);
    gp->set_pin_mode(8_pin, tos::pin_mode::in_pullup);

    auto tmr = open(tos::devs::timer<1>);
    auto alarm = open(tos::devs::alarm, *tmr);

    while (true)
    {
        alarm.sleep_for({ 1000 });
        tos::println(*usart, "Tick", (int)gp->read(8_pin));
    }
}

void tos_main()
{
    usart->options(
            tos::avr::usart_modes::async,
            tos::usart_parity::disabled,
            tos::usart_stop_bit::one);
    usart->enable();

    tos::launch(hello_task);
    tos::launch(tick_task);
}