//
// Created by fatih on 10/25/18.
//

#include <tos/ft.hpp>

#include <tos/semaphore.hpp>

#include <arch/drivers.hpp>
#include <tos/fixed_fifo.hpp>
#include <tos/mem_stream.hpp>
#include <tos/print.hpp>

void usart_setup(tos::stm32::gpio& g)
{
    using namespace tos::tos_literals;

#if defined(STM32L0)
    auto tx_pin = 9_pin;
    auto rx_pin = 10_pin;

    g.set_pin_mode(rx_pin, tos::pin_mode::in);

    gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO9);
    gpio_set_af(GPIOA, GPIO_AF7, GPIO9 | GPIO10);
#elif defined(STM32L4)
    auto tx_pin = 22_pin;
    auto rx_pin = 23_pin;

    g.set_pin_mode(rx_pin, tos::pin_mode::in);

    gpio_mode_setup(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO6);
    gpio_set_af(GPIOB, GPIO_AF7, GPIO6 | GPIO7);
#elif defined(STM32F1)
    auto tx_pin = 2_pin;
    auto rx_pin = 3_pin;

    g.set_pin_mode(rx_pin, tos::pin_mode::in);

    gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_50_MHZ,
                  GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO_USART2_TX);
#endif
}

void blink_task()
{
    using namespace tos;
    using namespace tos_literals;
    constexpr auto usconf = tos::usart_config()
        .add(115200_baud_rate)
        .add(usart_parity::disabled)
        .add(usart_stop_bit::one);

	auto g = tos::open(tos::devs::gpio);

    usart_setup(g);
    auto usart = tos::open(tos::devs::usart<0>, usconf);

    auto tmr = tos::open(tos::devs::timer<2>);
    auto alarm = tos::open(tos::devs::alarm, tmr);

    g.set_pin_mode(5_pin, tos::pin_mode::out);

	tos::println(usart, int(rcc_ahb_frequency), int(rcc_apb1_frequency));

    g.write(5_pin, tos::digital::high);
    while (true)
    {
        using namespace std::chrono_literals;
        alarm.sleep_for(1s);
        tos::println(usart, "l");
        g.write(5_pin, tos::digital::low);

        alarm.sleep_for(1s);
        tos::println(usart, "h");
        g.write(5_pin, tos::digital::high);
    }
}

void tos_main()
{
    tos::launch(tos::alloc_stack, blink_task);
}
