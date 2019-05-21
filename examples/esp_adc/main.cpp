//
// Created by fatih on 11/7/18.
//

#include <arch/drivers.hpp>
#include <tos/ft.hpp>
#include <tos/print.hpp>

void adc_main()
{
    using namespace tos::tos_literals;

    constexpr auto usconf = tos::usart_config()
            .add(115200_baud_rate)
            .add(tos::usart_parity::disabled)
            .add(tos::usart_stop_bit::one);

    auto usart = open(tos::devs::usart<0>, usconf);

    tos::esp82::adc a;

    auto timer = open(tos::devs::timer<0>);
    auto alarm = open(tos::devs::alarm, timer);

    while (true)
    {
        using namespace std::chrono_literals;
        alarm.sleep_for(500ms);
        auto x = a.read();
        tos::println(usart, int(x));
    }
}

void tos_main()
{
    tos::launch(tos::alloc_stack, adc_main);
}