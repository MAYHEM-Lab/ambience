//
// Created by fatih on 10/25/18.
//

#include <arch/drivers.hpp>
#include <tos/board.hpp>
#include <tos/ft.hpp>
#include <tos/print.hpp>
#include <tos/semaphore.hpp>

using bs = tos::bsp::board_spec;

void blink_task() {
    using namespace tos;
    using namespace tos_literals;

    auto g = tos::open(tos::devs::gpio);

    auto led_pin = tos::stm32::instantiate_pin(bs::led_pin);

    auto timer = open(devs::timer<2>);
    tos::alarm alarm(&timer);

    auto usart = bs::default_com::open();
    tos::println(usart, "Hello From Tos!");

    g.set_pin_mode(led_pin, tos::pin_mode::out);
    g.write(led_pin, tos::digital::low);
    while (true) {
        using namespace std::chrono_literals;
        g.write(led_pin, tos::digital::high);
        tos::println(usart, "Low");
        tos::this_thread::sleep_for(alarm, 1s);

        g.write(led_pin, tos::digital::low);
        tos::println(usart, "High");
        tos::this_thread::sleep_for(alarm, 1s);
    }
}

void tos_main() { tos::launch(tos::alloc_stack, blink_task); }
