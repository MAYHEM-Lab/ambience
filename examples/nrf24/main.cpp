//
// Created by fatih on 5/25/18.
//

#include <common/nrf24.hpp>
#include <tos/interrupt.hpp>
#include <tos/ft.hpp>
#include <tos/print.hpp>
#include <arch/avr/drivers.hpp>
#include <common/alarm.hpp>

void nrf_task()
{
    using namespace tos;
    using namespace tos::tos_literals;

    constexpr auto usconf = tos::usart_config()
            .add(115200_baud_rate)
            .add(usart_parity::disabled)
            .add(usart_stop_bit::one);

    auto usart = open(tos::devs::usart<0>, usconf);

    tos::println(*usart, "hello");

    auto spi = open(tos::devs::spi<0>, tos::spi_mode::master);
    spi->enable();

    auto g = open(tos::devs::gpio);

    tos::nrf24<decltype(g), decltype(spi)> nrf(g, spi, 8_pin, 10_pin, 2_pin);

    tos::println(*usart, "is connected:", nrf.is_connected());
    tos::println(*usart, "set_speed(250kbs):", nrf.set_speed(tos::nrf24_speeds::s_1_mbits));
    tos::println(*usart, "set_retries(15, 5):", nrf.set_retries(5, 15));

    nrf.set_channel({ 100 });
    tos::println(*usart, "channel:", nrf.get_channel().channel);

    nrf.set_addr_width(tos::nrf24_addr_width::w_5_bytes);
    tos::println(*usart, "width: ", (uint8_t)nrf.get_addr_width());

    uint8_t addr[] = { '1', 'N', 'o', 'd', 'e' };
    nrf.open_read_pipe(0, addr);

    while(true){
        tos::println(*usart, "psize", nrf.get_next_length());
        tos::this_thread::yield();
    }
}

void tos_main()
{
    tos::launch(tos::alloc_stack, nrf_task);
}