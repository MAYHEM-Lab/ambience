//
// Created by fatih on 10/25/18.
//

#include <arch/drivers.hpp>
#include <numeric>
#include <tos/fixed_fifo.hpp>
#include <tos/ft.hpp>
#include <tos/mem_stream.hpp>
#include <tos/periph/stm32f7_ltdc.hpp>
#include <tos/periph/stm32f7_sdram.hpp>
#include <tos/periph/stm32f7_dma2d.hpp>
#include <tos/print.hpp>
#include <tos/semaphore.hpp>

namespace boards {
struct stm32f7_disco {
    static constexpr int led_pin = ('i' - 'a') * 16 + 1;
    static constexpr int tx_pin = 9;
    static constexpr int rx_pin = 23;
};
} // namespace boards

constexpr auto board = boards::stm32f7_disco{};

#define SDRAM_BANK_ADDR ((uint32_t)0xC0000000)

tos::span<uint16_t> sdram_span() {
    return {(uint16_t*)SDRAM_BANK_ADDR, (uint16_t*)SDRAM_BANK_ADDR + 480*272};
}

void blink_task() {
    using namespace tos;
    using namespace tos_literals;

    auto led_pin = operator""_pin(board.led_pin);
    auto usart_rx_pin = operator""_pin(board.rx_pin);
    auto usart_tx_pin = operator""_pin(board.tx_pin);

    auto g = tos::open(tos::devs::gpio);

    auto ram_span = sdram_span();
    tos::stm32::f7::sdram ram;

    auto timer = open(devs::timer<2>);
    tos::alarm alarm(&timer);

    auto usart =
        open(devs::usart<1>, tos::uart::default_115200, usart_rx_pin, usart_tx_pin);
    tos::println(usart, "Hello From Tos!");

//    for (auto& i : sdram_span()) {
//        tos::println(usart, int(i));
//    }
//
//    std::iota(ram_span.begin(), ram_span.end(), 0);
//
//    for (auto& i : sdram_span()) {
//        tos::println(usart, int(i));
//    }

    tos::println(usart, "init display!");

    tos::stm32::f7::ltdc_layer layer;
    layer.set_window(0, 0, 480, 272);
    layer.set_blending(LTDC_BLENDING_FACTOR1_CA, LTDC_BLENDING_FACTOR2_CA);
    layer.set_pixel_format(LTDC_PIXEL_FORMAT_RGB565);
    layer.set_framebuffer(const_cast<uint16_t*>(sdram_span().data()));
//    layer.set_framebuffer(data);
    layer.set_alpha(255);
    layer.set_dimensions(480, 272);
    layer.set_back_color(0, 0, 0, 0);

    tos::stm32::f7::ltdc display(layer);
    tos::println(usart, "past!");

    g.set_pin_mode(led_pin, tos::pin_mode::out);

    g.write(led_pin, tos::digital::high);

    tos::stm32::f7::dma2d dma2d;
    tos::println(usart, "dma2d ok");

    while (true) {
        using namespace std::chrono_literals;
        dma2d.fill_rect(ram_span.data(), 0xFF0000FF, 480, 272);
        tos::this_thread::sleep_for(alarm, 1000ms);
        dma2d.fill_rect(ram_span.data(), 0xFFFF0000, 480, 272);
        tos::this_thread::sleep_for(alarm, 1000ms);
        tos::println(usart, "tick");
//        dma2d.fill_rect(sdram_span().data(), 0xF00F, 480, 272);
//        tos::this_thread::sleep_for(alarm, 100ms);
//        std::fill(sdram_span().begin(), sdram_span().end(), 0x0FF0);
//        std::fill(sdram_span().begin(), sdram_span().end(), 0xF00F);
    }
}

tos::stack_storage<4096> stack;
void tos_main() {
    tos::launch(stack, blink_task);
}
