//
// Created by Mehmet Fatih BAKIR on 11/05/2018.
//

#include <arch/drivers.hpp>
#include <common/alarm.hpp>
#include <tos/arch.hpp>
#include <tos/ft.hpp>
#include <tos/print.hpp>

void tick_task() {
    using namespace tos;
    using namespace tos::tos_literals;

    auto usart = open(tos::devs::usart<0>, tos::uart::default_9600);

    auto tmr = open(tos::devs::timer<1>);
    auto alarm = open(tos::devs::alarm, *tmr);

    tos::println(*usart, "Hello!");
    while (true) {
        tos::println(*usart, "yo");
        using namespace std::chrono_literals;
        tos::this_thread::sleep_for(alarm, 1s);
    }
}

void tos_main() {
    tos::launch(tos::alloc_stack, tick_task);
}