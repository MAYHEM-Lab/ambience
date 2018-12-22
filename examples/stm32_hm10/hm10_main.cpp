//
// Created by fatih on 12/19/18.
//

#include <common/hm-10.hpp>
#include <tos/ft.hpp>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>
#include <drivers/arch/stm32/drivers.hpp>
#include <drivers/common/usart.hpp>

void usart_setup(tos::stm32::gpio& g)
{
    using namespace tos::tos_literals;

    auto tx_pin = 2_pin;
    auto rx_pin = 3_pin;

    g.set_pin_mode(rx_pin, tos::pin_mode::in);
    //g.set_pin_mode(tx_pin, tos::pin_mode::out);
    //g.write(tx_pin, tos::digital::high);

    gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_50_MHZ,
                  GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO_USART2_TX);
}

void usart3_setup(tos::stm32::gpio& g)
{
    using namespace tos::tos_literals;

    rcc_periph_clock_enable(RCC_AFIO);
    AFIO_MAPR |= AFIO_MAPR_USART3_REMAP_PARTIAL_REMAP;

    auto tx_pin = 42_pin;
    auto rx_pin = 43_pin;

    g.set_pin_mode(rx_pin, tos::pin_mode::in);
    gpio_set_mode(GPIO_BANK_USART3_PR_TX, GPIO_MODE_OUTPUT_50_MHZ,
                  GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO_USART3_PR_TX);
}

auto hm10_task = [](void*){
    auto g = tos::open(tos::devs::gpio);

    usart_setup(g);
    auto usart = tos::open(tos::devs::usart<1>, tos::uart::default_9600);

    usart3_setup(g);
    auto lora_uart = tos::open(tos::devs::usart<2>, tos::uart::default_9600);

    auto tmr = tos::open(tos::devs::timer<2>);
    auto alarm = tos::open(tos::devs::alarm, tmr);

    using namespace std::chrono_literals;
    alarm.sleep_for(100ms);

    tos::println(usart, "hello");

    auto hm10 = tos::open(tos::devs::hm10, &lora_uart);
    hm10->test(alarm);

    tos::println(usart, hm10.get_address());
    tos::println(usart, hm10.notifications_enabled());

    while (true)
    {
        char b[1];
        tos::print(hm10, hm10->read(b));
    }
};

void tos_main()
{
    tos::launch(hm10_task);
}