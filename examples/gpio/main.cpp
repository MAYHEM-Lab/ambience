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

using namespace tos;
using namespace tos::tos_literals;

constexpr auto usconf = tos::usart_config()
        .add(115200_baud_rate)
        .add(usart_parity::disabled)
        .add(usart_stop_bit::one);

auto usart = open(tos::devs::usart<0>, usconf);

void hello_task(void*)
{
    using namespace tos::tos_literals;
    gp->set_pin_mode(13_pin, tos::pin_mode::out);
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

void tick_task(void*)
{
    using namespace tos::tos_literals;
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
    tos::launch(hello_task);
    tos::launch(tick_task);
}