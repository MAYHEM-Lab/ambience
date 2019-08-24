//
// Created by fatih on 10/8/18.
//

#include <arch/drivers.hpp>
#include <tos/ft.hpp>
#include <tos/print.hpp>
#include <tos/version.hpp>

void spi_task() {
    auto g = tos::open(tos::devs::gpio);
    auto spi = tos::open(tos::devs::spi<0>, tos::spi_mode::master, g);

    auto tmr = tos::open(tos::devs::timer<0>);
    auto alarm = tos::open(tos::devs::alarm, tmr);

    using namespace tos::tos_literals;

    auto usart = open(tos::devs::usart<0>, tos::uart::default_9600);

    tos::print(usart, "\n\n\n\n\n\n");
    tos::println(usart, tos::platform::board_name);
    tos::println(usart, tos::vcs::commit_hash);

    while (true) {
        using namespace std::chrono_literals;
        alarm.sleep_for(1s);
        tos::println(usart, "tick!");
        spi.exchange(0xDA);

        uint8_t buf[] = "hello world";

        spi.exchange_many(buf);
    }
}

void tos_main() {
    tos::launch(tos::alloc_stack, spi_task);
}