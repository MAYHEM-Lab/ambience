//
// Created by fatih on 5/25/19.
//

#include <tos/ft.hpp>
#include <arch/drivers.hpp>
#include <common/mfrc522.hpp>
#include <tos/print.hpp>
#include <common/spi.hpp>

auto delay = [](std::chrono::microseconds us) {
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

void blink_task()
{
    using namespace tos::tos_literals;

    rcc_periph_clock_enable(RCC_GPIOA);
    rcc_periph_clock_enable(RCC_GPIOB);

    rcc_periph_clock_enable(RCC_AFIO);

    gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_50_MHZ,
                  GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO5 |
                                                  GPIO7);

    gpio_set_mode(GPIOA, GPIO_MODE_INPUT, GPIO_CNF_INPUT_FLOAT,
                  GPIO6);

    auto g = tos::open(tos::devs::gpio);

    usart_setup(g);
    auto usart = tos::open(tos::devs::usart<1>, tos::uart::default_9600);
    tos::println(usart, "hello");

    auto cs = 22_pin;

    auto reset_pin = 39_pin;

    g.set_pin_mode(reset_pin, tos::pin_mode::out);
    g.write(reset_pin, tos::digital::low);

    g.set_pin_mode(cs, tos::pin_mode::out);
    g.write(cs, tos::digital::high);

    auto tmr = tos::open(tos::devs::timer<2>);
    auto alarm = tos::open(tos::devs::alarm, tmr);

    tos::stm32::spi spi(tos::stm32::detail::spis[0]);

    while (true)
    {
        tos::spi_transaction<decltype(spi)> tr{spi, g, cs};
        tos::println(*usart, "exchange", int(tr->exchange(0)));
        using namespace std::chrono_literals;
        tos::this_thread::sleep_for(alarm, 1s);
    }

    tos::this_thread::block_forever();

    tos::mfrc522<decltype(&spi)> rfid{ &spi, &g, cs, reset_pin };
    rfid.init(delay);

    tos::println(usart, "gain", int(rfid.PCD_GetAntennaGain()));
    tos::println(usart, "fw version", int(rfid.get_fw_version()));

    g.write(5_pin, tos::digital::high);
    while (true)
    {
        tos::println(usart, rfid.PICC_IsNewCardPresent());
        using namespace std::chrono_literals;
        tos::this_thread::sleep_for(alarm, 100ms);
    }
}

void tos_main()
{
    tos::launch(tos::alloc_stack, blink_task);
}