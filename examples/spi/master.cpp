//
// Created by fatih on 4/15/18.
//

#include <arch/drivers.hpp>
#include <common/spi.hpp>
#include <tos/ft.hpp>
#include <tos/print.hpp>

void master_task() {
    using namespace tos::tos_literals;
    auto spi = open(tos::devs::spi<0>, tos::spi_mode::master);

    constexpr auto usconf = tos::usart_config()
                                .add(19200_baud_rate)
                                .add(tos::usart_parity::disabled)
                                .add(tos::usart_stop_bit::one);

    auto usart = open(tos::devs::usart<0>, usconf);

    tos::println(usart, "Hi from master!");

    while (true) {
        uint8_t c;
        auto read = usart.read(tos::monospan(c));
        spi.exchange(read);
    }
}

void tos_main() {
    tos::launch(tos::alloc_stack, master_task);
}