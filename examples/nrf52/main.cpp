//
// Created by Mehmet Fatih BAKIR on 29/03/2018.
//

#include <arch/drivers.hpp>
#include <common/alarm.hpp>
#include <tos/board.hpp>
#include <tos/ft.hpp>
#include <tos/print.hpp>

using bs = tos::bsp::board_spec;

auto blink_task = [] {
    using namespace tos;
    using namespace tos_literals;

    auto led_pin = nrf52::instantiate_pin(bs::led_pin);
    auto g = open(tos::devs::gpio);
    g->set_pin_mode(led_pin, pin_mode::out);

    auto usart = bs::default_com::open();

    tos::println(usart, "hello");

    auto timer = open(tos::devs::timer<1>);
    tos::alarm alarm(&timer);

    while (true) {
        using namespace std::chrono_literals;

        g->write(led_pin, digital::low);
        tos::println(usart, "On");
        tos::this_thread::sleep_for(alarm, 1s);

        g->write(led_pin, digital::high);
        tos::println(usart, "Off");
        tos::this_thread::sleep_for(alarm, 1s);
    }

    tos::this_thread::block_forever();
};

void tos_main() {
    tos::launch(tos::alloc_stack, blink_task);
}
