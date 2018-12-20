//
// Created by fatih on 4/15/18.
//

#include <common/spi.hpp>
#include <tos/ft.hpp>
#include <tos/print.hpp>
#include <arch/avr/drivers.hpp>

void master_task(void*)
{
    using namespace tos::tos_literals;
    auto spi = open(tos::devs::spi<0>, tos::spi_mode::master);
    spi.enable();

    constexpr auto usconf = tos::usart_config()
            .add(19200_baud_rate)
            .add(tos::usart_parity::disabled)
            .add(tos::usart_stop_bit::one);

    auto usart = open(tos::devs::usart<0>, usconf);

    tos::println(usart, "Hi from master!");

    while (true)
    {
        char c[1];
        spi.exchange(usart.read(c)[0]);
    }
}

void tos_main()
{
    tos::launch(master_task);
}