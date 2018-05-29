//
// Created by fatih on 5/25/18.
//

#include <drivers/common/nrf24.hpp>
#include <drivers/arch/avr/usart.hpp>
#include <tos/interrupt.hpp>
#include <tos/ft.hpp>
#include <tos/print.hpp>
#include <drivers/arch/avr/timer.hpp>
#include <drivers/common/alarm.hpp>

void nrf_task()
{
    using namespace tos::tos_literals;

    auto usart = tos::open(tos::devs::usart<0>, 19200_baud_rate);
    usart->options(
            tos::usart_modes::async,
            tos::usart_parity::disabled,
            tos::usart_stop_bit::one);
    usart->enable();

    tos::println(*usart, "hello");

    auto spi = open(tos::devs::spi<0>, tos::spi_mode::master);
    spi->enable();

    tos::nrf24 nrf(8_pin, 10_pin, 2_pin);

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

int main()
{
    tos::enable_interrupts();

    tos::launch(nrf_task);

    while(true)
    {
        tos::schedule();
    }
}