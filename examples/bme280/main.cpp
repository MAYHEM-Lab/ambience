//
// Created by fatih on 4/4/19.
//

#include <arch/stm32/drivers.hpp>
#include <common/bme280/bme280.h>
#include <common/bme280.hpp>
#include <tos/print.hpp>
#include <tos/expected.hpp>

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

void bme_task()
{
    using namespace tos::tos_literals;

    auto g = tos::open(tos::devs::gpio);

    usart_setup(g);
    auto usart = tos::open(tos::devs::usart<1>, tos::uart::default_9600);
    tos::println(usart, "hello");

    rcc_periph_clock_enable(RCC_AFIO);
    AFIO_MAPR |= AFIO_MAPR_I2C1_REMAP;

    tos::stm32::twim t { 24_pin, 25_pin };

    using namespace tos::bme280;
    bme280 b{ {BME280_I2C_ADDR_PRIM}, &t, delay };
    b.set_config();
    b.enable();

    tos::println(usart, "Temperature, Pressure, Humidity");
    while(true)
    {
        using namespace std::chrono_literals;
        delay(70ms);
        with(b.read(components::all), [&](auto& comp_data){
            tos::println(usart, int(comp_data.temperature), int(comp_data.pressure), int(comp_data.humidity));
        }, tos::ignore);
    }
}

void tos_main()
{
    tos::launch(tos::alloc_stack, bme_task);
}