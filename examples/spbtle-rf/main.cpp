//
// Created by fatih on 7/19/19.
//

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <arch/drivers.hpp>
#include <tos/print.hpp>
#include <SPBTLE_RF.h>

void usart_setup(tos::stm32::gpio &g) {
    using namespace tos::tos_literals;

#if defined(STM32L0)
    auto tx_pin = 9_pin;
    auto rx_pin = 10_pin;

    g.set_pin_mode(rx_pin, tos::pin_mode::in);

    gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO9);
    gpio_set_af(GPIOA, GPIO_AF7, GPIO9 | GPIO10);
#elif defined(STM32L4)
    //auto tx_pin = 22_pin;
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

void ble_task() {
    using namespace tos::tos_literals;
    rcc_periph_clock_enable(RCC_GPIOC);
    gpio_mode_setup(GPIOC, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO10 | GPIO11 | GPIO12);
    gpio_set_af(GPIOC, GPIO_AF6, GPIO10 | GPIO11 | GPIO12);

    auto reset = 8_pin;
    auto cs_pin = 61_pin;
    auto exti_pin = 70_pin;

    auto g = tos::open(tos::devs::gpio);

    auto timer = open(tos::devs::timer<2>);
    auto alarm = open(tos::devs::alarm, timer);
    auto erased_alarm = tos::erase_alarm(&alarm);

    constexpr auto usconf = tos::usart_config()
            .add(115200_baud_rate)
            .add(tos::usart_parity::disabled)
            .add(tos::usart_stop_bit::one);

    usart_setup(g);
    auto usart = tos::open(tos::devs::usart<0>, usconf);
    tos::println(usart, "hello");
    using namespace std::chrono_literals;
    tos::println(usart, "yoo");

    tos::stm32::exti e;

    tos::stm32::spi s(tos::stm32::detail::spis[2]);
    s.set_8_bit_mode();

    spbtle_rf bl(&s, &e, *erased_alarm, cs_pin, exti_pin, reset);

    tos::println(usart, "began");

    auto build_number = bl.get_fw_id();
    if (!build_number)
    {
        tos::println(usart, "can't communicate");
        tos::this_thread::block_forever();
    }

    tos::println(usart, bool(build_number), int(force_get(build_number).build_number));

    auto gatt = bl.initialize_gatt();
    if (!gatt)
    {
        tos::println(usart, "gatt failed");
        tos::this_thread::block_forever();
    }

    auto gap = bl.initialize_gap(force_get(gatt), "Tos BLE");
    tos::println(usart, "gap init'd");

    tos::spbtle::advertising adv(1s, "Tos BLE");
    tos::println(usart, "disc started");

    tos::this_thread::block_forever();
}

static tos::stack_storage<2048> sstore;

void tos_main() {
    tos::launch(sstore, ble_task);
}
