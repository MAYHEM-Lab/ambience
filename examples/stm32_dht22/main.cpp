//
// Created by fatih on 10/25/18.
//

#include <tos/ft.hpp>

#include <tos/semaphore.hpp>

#include <drivers/arch/stm32/drivers.hpp>
#include <tos/fixed_fifo.hpp>
#include <tos/mem_stream.hpp>
#include <tos/print.hpp>
#include <drivers/common/dht22.hpp>

void usart_setup(tos::stm32::gpio& g)
{
    using namespace tos::tos_literals;

    auto tx_pin = 2_pin;
    auto rx_pin = 3_pin;

    g.set_pin_mode(rx_pin, tos::pin_mode::in);

    gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_50_MHZ,
                  GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO_USART2_TX);
}

auto delay = [](std::chrono::microseconds us) {
    uint32_t end = (us.count() * (rcc_ahb_frequency / 1'000'000)) / 13.3;
    for (volatile int i = 0; i < end; ++i)
    {
        __asm__ __volatile__ ("nop");
    }
};

void dht_task(void*)
{
	using namespace tos::tos_literals;

	auto g = tos::open(tos::devs::gpio);

    usart_setup(g);
    auto usart = tos::open(tos::devs::usart<1>, tos::uart::default_9600);

    tos::println(usart, int(rcc_ahb_frequency), int(rcc_apb1_frequency));
	auto dht = tos::make_dht(g, delay);
	auto dht_pin = 8_pin;

    while (true)
    {
        using namespace std::chrono_literals;
        delay(1s);

        auto res = dht.read(dht_pin);
        tos::println(usart, int(res), int(dht.temperature), int(dht.humidity));
    }
}

void tos_main()
{
    tos::launch(dht_task);
}
