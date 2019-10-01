//
// Created by Mehmet Fatih BAKIR on 29/03/2018.
//

#include <arch/drivers.hpp>
#include <common/alarm.hpp>

#include <tos/ft.hpp>
#include <tos/print.hpp>
#include <tos/fixed_fifo.hpp>

void gap_params_init();
void gatt_init();

auto blink_task = []
{
    using namespace tos;
    using namespace tos_literals;
    constexpr auto usconf = tos::usart_config()
        .add(115200_baud_rate)
        .add(usart_parity::disabled)
        .add(usart_stop_bit::one);

    auto g = open(tos::devs::gpio);
    g->set_pin_mode(13_pin, pin_mode::out);

    auto usart = open(tos::devs::usart<0>, usconf, 6_pin, 8_pin);

    tos::println(usart, "hello");

    auto timer = open(tos::devs::timer<1>);
    auto alarm = open(tos::devs::alarm, timer);

    while (true)
    {
        using namespace std::chrono_literals;

        g->write(13_pin, digital::low);
        tos::println(usart, "On");
        alarm.sleep_for(1s);

        g->write(13_pin, digital::high);
        tos::println(usart, "Off");
        alarm.sleep_for(1s);
    }

    tos::this_thread::block_forever();
};

void TOS_EXPORT tos_main()
{
    tos::launch(tos::alloc_stack, blink_task);
}
