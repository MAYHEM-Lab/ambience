//
// Created by fatih on 10/25/18.
//

#include <arch/drivers.hpp>
#include <tos/board.hpp>
#include <tos/ft.hpp>
#include <tos/print.hpp>
#include <tos/semaphore.hpp>

using bs = tos::bsp::board_spec;

void slave_task() {
    using namespace tos;
    using namespace tos_literals;

    auto spi = bs::spi1s::open();

    // auto g = tos::open(tos::devs::gpio);

    auto usart = bs::default_com::open();
    tos::println(usart, "Hi from slave!");

    std::array<uint8_t, 10> send_buffer = {'f', 'r', 'o', 'm', ' ', 's', 'l', 'a', 'v', 'e'};
    std::array<uint8_t, 10> recieve_buffer;

    // send 10 bytes to master
    spi.exchange(recieve_buffer, send_buffer);
    tos::println(usart, "Just sent to master.");

    // print buffer to usart
    tos::print(usart, "Recieved buffer: ");
    usart.write(recieve_buffer);
    tos::println(usart, "");

    tos::this_thread::block_forever();
}

void tos_main() {
    tos::launch(tos::alloc_stack, slave_task);
}
