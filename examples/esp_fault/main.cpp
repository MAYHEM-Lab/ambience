//
// Created by fatih on 8/27/18.
//

#include <drivers/arch/lx106/drivers.hpp>
#include <tos/ft.hpp>
#include <tos/print.hpp>
#include <tos/version.hpp>

extern "C" void lwip_init() {}

extern int* some_var;
void task(void*)
{
    using namespace tos::tos_literals;

    constexpr auto usconf = tos::usart_config()
            .add(115200_baud_rate)
            .add(tos::usart_parity::disabled)
            .add(tos::usart_stop_bit::one);

    auto usart = open(tos::devs::usart<0>, usconf);

    tos::print(usart, "\n\n\n\n\n\n");
    tos::println(usart, tos::platform::board_name);
    tos::println(usart, tos::vcs::commit_hash);

    *some_var = 5;

    while (true)
    {
        tos::println(usart, "hello");
    }
}

void tos_main()
{
    tos::launch(task);
}