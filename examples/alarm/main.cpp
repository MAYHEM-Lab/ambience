//
// Created by Mehmet Fatih BAKIR on 11/05/2018.
//

#include <arch/avr/drivers.hpp>
#include <tos/ft.hpp>
#include <tos/print.hpp>
#include <tos/arch.hpp>
#include <common/alarm.hpp>
#include <tos/devices.hpp>
#include <tos/waitable.hpp>

void tick_task()
{
    using namespace tos;
    using namespace tos::tos_literals;

    constexpr auto usconf = tos::usart_config()
            .add(115200_baud_rate)
            .add(usart_parity::disabled)
            .add(usart_stop_bit::one);

    auto usart = open(tos::devs::usart<0>, usconf);

    auto tmr = open(tos::devs::timer<1>);
    auto alarm = open(tos::devs::alarm, *tmr);

    tos::println(*usart, "Hello!");
    while (true)
    {
        tos::println(*usart, "yo");
        using namespace std::chrono_literals;
        alarm.sleep_for(1s);
    }
}

void tos_main()
{
    tos::launch(tos::alloc_stack, tick_task);
}