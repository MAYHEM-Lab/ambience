//
// Created by fatih on 10/25/18.
//

#include <arch/drivers.hpp>
#include <tos/fixed_fifo.hpp>
#include <tos/ft.hpp>
#include <tos/mem_stream.hpp>
#include <tos/print.hpp>
#include <tos/semaphore.hpp>

namespace boards {
struct stm32f7_disco {
    static constexpr int led_pin = ('i' - 'a') * 16 + 1;
    static constexpr int tx_pin = 9;
    static constexpr int rx_pin = 23;
};
struct stm32l4_disco {
    static constexpr int led_pin = 5;
    static constexpr int tx_pin = 22;
    static constexpr int rx_pin = 23;
};
}

constexpr auto board = boards::stm32f7_disco{};

void blink_task() {
    using namespace tos;
    using namespace tos_literals;

    auto led_pin = operator""_pin(board.led_pin);
    auto usart_rx_pin = operator""_pin(board.rx_pin);
    auto usart_tx_pin = operator""_pin(board.tx_pin);

    auto g = tos::open(tos::devs::gpio);

    auto timer = open(devs::timer<2>);
    tos::alarm alarm(&timer);

    auto usart =
        open(devs::usart<1>, tos::uart::default_115200, usart_rx_pin, usart_tx_pin);
    tos::println(usart, "Hello From Tos!");

    g.set_pin_mode(led_pin, tos::pin_mode::out);

    g.write(led_pin, tos::digital::high);
    while (true) {
        using namespace std::chrono_literals;
        g.write(led_pin, tos::digital::low);
        tos::println(usart, "Low");
        tos::this_thread::sleep_for(alarm, 1s);

        g.write(led_pin, tos::digital::high);
        tos::println(usart, "High");
        tos::this_thread::sleep_for(alarm, 1s);
    }
}

void tos_main() { tos::launch(tos::alloc_stack, blink_task); }
