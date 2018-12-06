//
// Created by fatih on 12/5/18.
//

#include <common/hx711.hpp>
#include <tos/ft.hpp>
#include <arch/stm32/drivers.hpp>
#include <tos/print.hpp>
#include <tos/devices.hpp>

auto delay = [](std::chrono::microseconds us) NO_INLINE {
    uint32_t end = (us.count() * (rcc_ahb_frequency / 1'000'000)) / 13.3;
    for (volatile int i = 0; i < end; ++i)
    {
        __asm__ __volatile__ ("nop");
    }
};

void usart_setup(tos::stm32::gpio& g)
{
    using namespace tos::tos_literals;

    auto tx_pin = 2_pin;
    auto rx_pin = 3_pin;

    g.set_pin_mode(rx_pin, tos::pin_mode::in);

    gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_50_MHZ,
                  GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO_USART2_TX);
}

void hx711_task(void*)
{
    using namespace tos::tos_literals;
    using namespace std::chrono_literals;

    auto g = tos::open(tos::devs::gpio);

    usart_setup(g);
    auto usart = tos::open(tos::devs::usart<1>, tos::uart::default_9600);

    tos::println(usart, "hello");
    tos::println(usart, int(rcc_ahb_frequency), int(rcc_apb1_frequency));

    auto data_pin = 9_pin;
    auto clk_pin = 8_pin;

    auto hx = tos::open(tos::devs::hx711, g, delay, clk_pin, data_pin);

    while (true)
    {
        using namespace std::chrono_literals;
        delay(1s);

        auto res = hx.get_val();

        tos::println(usart, int(res));
    }
}

void tos_main()
{
    tos::launch(hx711_task);
}