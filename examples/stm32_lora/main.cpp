//
// Created by fatih on 12/12/18.
//

#include <common/rn2903/sys.hpp>
#include <common/rn2903/rn2903.hpp>
#include <tos/ft.hpp>
#include <tos/print.hpp>
#include <arch/drivers.hpp>

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
    //g.set_pin_mode(tx_pin, tos::pin_mode::out);
    //g.write(tx_pin, tos::digital::high);

    gpio_set_mode(GPIOC, GPIO_MODE_OUTPUT_50_MHZ,
                  GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO_USART3_PR_TX);

}

void lora_task()
{
    auto g = tos::open(tos::devs::gpio);

    usart_setup(g);
    auto usart = tos::open(tos::devs::usart<1>, tos::uart::default_9600);

    using namespace tos::tos_literals;
    constexpr auto lora_uart_config = tos::usart_config()
            .add(57600_baud_rate)
            .add(tos::usart_parity::disabled)
            .add(tos::usart_stop_bit::one);

    usart3_setup(g);
    auto lora_uart = tos::open(tos::devs::usart<2>, lora_uart_config);

    auto tmr = tos::open(tos::devs::timer<2>);
    tos::alarm alarm(&tmr);

    using namespace std::chrono_literals;
    tos::println(usart, "hello world");

    tos::println(lora_uart, "sys reset");
    static char buf[128];
    auto r = lora_uart.read(buf, alarm, 10s);
    tos::println(usart, r.size(), r);

    auto lora = tos::open(tos::devs::rn2903, lora_uart, alarm);
    auto res = lora.nvm_get({ 300 });

    if (!res)
    {
        tos::println(usart, "nvm_get failed");
        tos::this_thread::block_forever();
        return;
    }

    auto num = tos::force_get(res);
    tos::println(usart, "got:", int(num));

    auto conn = lora.join_otaa();
    tos::println(usart, "got:", int(force_get(conn)));

    tos::this_thread::block_forever();
}

void tos_main()
{
    //static char lora_stack[2048];
    tos::launch(tos::alloc_stack, lora_task);
}