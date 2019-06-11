//
// Created by fatih on 6/7/19.
//

#include <arch/drivers.hpp>
#include <tos/ft.hpp>
#include <tos/print.hpp>

using namespace tos;
using namespace tos::stm32;
using namespace tos::tos_literals;

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

void wifi_task()
{
    rcc_periph_clock_enable(RCC_GPIOB);
    rcc_periph_clock_enable(RCC_GPIOC);
    gpio_mode_setup(GPIOC, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO10 | GPIO11 | GPIO12);
    gpio_set_af(GPIOC, GPIO_AF6, GPIO10 | GPIO11 | GPIO12);

    auto reset_pin = 72_pin;
    auto cs_pin = 64_pin;
    auto dr_pin = 65_pin;
    auto wakeup_pin = 29_pin;
    auto boot0_pin = 28_pin;

    auto g = tos::open(tos::devs::gpio);

    g->set_pin_mode(reset_pin, tos::pin_mode::out);
    g->write(reset_pin, tos::digital::high); // active low

    g->set_pin_mode(cs_pin, tos::pin_mode::out);
    g->write(cs_pin, tos::digital::high);

    g->set_pin_mode(wakeup_pin, tos::pin_mode::out);
    g->write(wakeup_pin, tos::digital::low);

    g->set_pin_mode(dr_pin, tos::pin_mode::in);

    auto timer = open(tos::devs::timer<2>);
    auto alarm = open(tos::devs::alarm, timer);

    constexpr auto usconf = tos::usart_config()
        .add(115200_baud_rate)
        .add(usart_parity::disabled)
        .add(usart_stop_bit::one);

    usart_setup(g);
    auto usart = tos::open(tos::devs::usart<0>, usconf);
    tos::println(usart, "hello");

    spi s(stm32::detail::spis[2]);
    s.set_16_bit_mode();

    using namespace std::chrono_literals;
    g->write(reset_pin, tos::digital::low);
    alarm->sleep_for(10ms);
    g->write(reset_pin, tos::digital::high);
    alarm->sleep_for(500ms);

    tos::println(usart, "xchg");
    g->write(cs_pin, digital::low);
    auto res = s.exchange16(0x0a0a);
    auto res2 = s.exchange16(0x0a0a);
    auto res3 = s.exchange16(0x0a0a);
    g->write(cs_pin, digital::high);
    tos::println(usart, "got", int(res & 0xFF), int(res >> 8));
    tos::println(usart, "got", int(res2 & 0xFF), int(res2 >> 8));
    tos::println(usart, "got", int(res3 & 0xFF), int(res3 >> 8));

    uint16_t scan[] = {('Z' | ('5' << 8)), ('\r' | ('\n' << 8))};

    g->write(cs_pin, digital::low);
    s.exchange16(scan[0]);
    s.exchange16(scan[1]);
    g->write(cs_pin, digital::high);

    while (g->read(dr_pin));

    g->write(cs_pin, digital::low);
    for (int i = 0; i < 11; ++i)
    {
        auto c = s.exchange16(0x0a0a);
        char buf[2];
        memcpy(buf, &c, 2);
        tos::println(usart, buf);
        alarm->sleep_for(10ms);
    }
    g->write(cs_pin, digital::high);

    tos::this_thread::block_forever();
}

void tos_main()
{
	tos::launch(tos::alloc_stack, wifi_task);
}