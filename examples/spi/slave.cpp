//
// Created by fatih on 4/15/18.
//

#include <common/spi.hpp>
#include <arch/drivers.hpp>
#include <tos/ft.hpp>
#include <tos/print.hpp>

void slave_task()
{
    using namespace tos::tos_literals;
    auto spi = open(tos::devs::spi<0>, tos::spi_mode::slave);
    spi.enable();

    constexpr auto usconf = tos::usart_config()
            .add(9600_baud_rate)
            .add(tos::usart_parity::disabled)
            .add(tos::usart_stop_bit::one);

    auto usart = open(tos::devs::usart<0>, usconf);

    tos::println(usart, "Hi from slave!");

    while (true)
    {
        char c = spi.exchange(42);
        tos::println(usart, c);
    }
}

void tos_main()
{
    tos::launch(tos::alloc_stack, slave_task);
}