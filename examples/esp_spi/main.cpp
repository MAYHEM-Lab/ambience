//
// Created by fatih on 10/8/18.
//

#include <lwip/init.h>
#include <tos/ft.hpp>
#include <arch/drivers.hpp>
#include <tos/print.hpp>
#include <tos/version.hpp>

void spi_task()
{
    if (tos::this_thread::get_id().id == 3) { lwip_init(); }

    auto g = tos::open(tos::devs::gpio);
    tos::esp82::spi s{g};

    auto tmr = tos::open(tos::devs::timer<0>);
    auto alarm = tos::open(tos::devs::alarm, tmr);

    using namespace tos::tos_literals;

    constexpr auto usconf = tos::usart_config()
            .add(115200_baud_rate)
            .add(tos::usart_parity::disabled)
            .add(tos::usart_stop_bit::one);

    auto usart = open(tos::devs::usart<0>, usconf);

    tos::print(usart, "\n\n\n\n\n\n");
    tos::println(usart, tos::platform::board_name);
    tos::println(usart, tos::vcs::commit_hash);

    while (true)
    {
        using namespace std::chrono_literals;
        alarm.sleep_for(1000ms);
        tos::println(usart, "tick!");
        s.exchange(0xDA);

        uint8_t buf[] = "hello world";

        s.exchange_many(buf);
    }
}

void tos_main()
{
    tos::launch(tos::alloc_stack, spi_task);
}