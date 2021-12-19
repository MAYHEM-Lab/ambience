//
// Created by fatih on 10/13/19.
//

#include <arch/drivers.hpp>
#include <common/clock.hpp>
#include <tos/print.hpp>
#include <tos/ft.hpp>

void clock_main() {
    auto timer = tos::open(tos::devs::timer<2>);

    tos::clock clk(&timer);

    using namespace tos::tos_literals;
    auto usart = tos::open(tos::devs::usart<1>, tos::uart::default_9600, 23_pin, 22_pin);
    tos::println(usart, "hello world");

    while (true)
    {
        tos::println(usart, int(clk.now().time_since_epoch().count()));
    }
}

void tos_main() {
    tos::launch(tos::alloc_stack, clock_main);
}