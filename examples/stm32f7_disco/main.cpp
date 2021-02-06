//
// Created by fatih on 10/25/18.
//

#include "arch/gpio.hpp"
#include <arch/drivers.hpp>
#include <numeric>
#include <tos/board.hpp>
#include <tos/fixed_fifo.hpp>
#include <tos/ft.hpp>
#include <tos/mem_stream.hpp>
#include <tos/periph/stm32f7_dma2d.hpp>
#include <tos/periph/stm32f7_ltdc.hpp>
#include <tos/periph/stm32f7_sdram.hpp>
#include <tos/print.hpp>
#include <tos/semaphore.hpp>

using bs = tos::bsp::board_spec;

#define SDRAM_BANK_ADDR ((uint32_t)0xC0000000)

tos::span<uint16_t> sdram_span() {
    return {(uint16_t*)SDRAM_BANK_ADDR, (uint16_t*)SDRAM_BANK_ADDR + 480*272};
}

void blink_task() {
    using namespace tos;
    using namespace tos_literals;

    auto g = tos::open(tos::devs::gpio);

    auto ram_span = sdram_span();
    tos::stm32::f7::sdram ram;

    auto timer = open(devs::timer<2>);
    tos::alarm alarm(&timer);

    auto usart = bs::default_com::open();
    tos::println(usart, "Hello From Tos!");

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

    auto led_pin = tos::stm32::instantiate_pin(bs::led_pin);
    g.set_pin_mode(led_pin, tos::pin_mode::out);

    g.write(led_pin, tos::digital::high);

    tos::stm32::f7::dma2d dma2d;
    tos::println(usart, "dma2d ok");

    while (true) {
        using namespace std::chrono_literals;
        dma2d.fill_rect(
            ram_span.data(),
            480,
            tos::gfx2::rgb8{255, 0, 128},
            tos::gfx2::rectangle{tos::gfx2::point{100, 100}, tos::gfx2::size{100, 100}});
        tos::this_thread::sleep_for(alarm, 330ms);

        dma2d.fill_rect(
            ram_span.data(),
            480,
            tos::gfx2::rgb8{128, 255, 128},
            tos::gfx2::rectangle{tos::gfx2::point{100, 100}, tos::gfx2::size{100, 100}});
        tos::this_thread::sleep_for(alarm, 330ms);
    }
}

tos::stack_storage<4096> stack;
void tos_main() {
    tos::launch(stack, blink_task);
}
