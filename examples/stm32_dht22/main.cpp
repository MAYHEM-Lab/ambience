//
// Created by fatih on 10/25/18.
//

#include <tos/ft.hpp>

#include <tos/semaphore.hpp>

#include <arch/drivers.hpp>
#include <tos/fixed_fifo.hpp>
#include <tos/mem_stream.hpp>
#include <tos/print.hpp>
#include <common/dht22.hpp>

void usart_setup(tos::stm32::gpio& g)
{
    using namespace tos::tos_literals;

    auto tx_pin = 2_pin;
    auto rx_pin = 10_pin;

    g.set_pin_mode(rx_pin, tos::pin_mode::in);

    gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_50_MHZ,
                  GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO_USART1_TX);
}

auto delay = [](std::chrono::microseconds us) {
    uint32_t end = (us.count() * (rcc_ahb_frequency / 1'000'000)) / 13.3;
    for (volatile int i = 0; i < end; ++i)
    {
        __asm__ __volatile__ ("nop");
    }
};

void dht_task()
{
	using namespace tos::tos_literals;

	auto g = tos::open(tos::devs::gpio);

    usart_setup(g);
    auto usart = tos::open(tos::devs::usart<0>, tos::uart::default_9600);

    //tos::println(usart, int(rcc_ahb_frequency), int(rcc_apb1_frequency));
	auto dht = tos::make_dht(g, delay);
	auto dht_pin = 1_pin;

	tos::stm32::adc a { tos::stm32::detail::adcs };

	uint8_t x[1];
	x[0] = 16;
	a.set_channels(x);

	constexpr auto v25 = 1700;
	constexpr auto slope = 2;

	auto read_c = [&]{
	    float v = a.read();
	    return (v25 - v) / slope + 25;
	};

    while (true)
    {
        using namespace std::chrono_literals;

        auto res = dht.read(dht_pin);
        tos::println(usart, int(dht.temperature * 1000), int(read_c() * 1000));
        
        delay(60s);
    }
}

void tos_main()
{
    tos::launch(tos::alloc_stack, dht_task);
}
