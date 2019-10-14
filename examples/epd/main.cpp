//
// Created by fatih on 5/21/19.
//

#include <arch/drivers.hpp>
#include <common/epd/gde021a1.hpp>
#include <tos/fixed_fifo.hpp>
#include <tos/ft.hpp>
#include <tos/gfx/canvas.hpp>
#include <tos/gfx/text.hpp>
#include <tos/mem_stream.hpp>
#include <tos/print.hpp>
#include <tos/semaphore.hpp>

auto delay = [](std::chrono::microseconds us) { HAL_Delay(us.count() / 1000); };

void blink_task() {
    using namespace tos::tos_literals;

    auto chip_select = 15_pin;
    auto data_command = 27_pin;

    auto power_pin = 26_pin; // Active low

    auto g = tos::open(tos::devs::gpio);

    g.set_pin_mode(power_pin, tos::pin_mode::out);
    g.write(power_pin, tos::digital::low);

    g.set_pin_mode(chip_select, tos::pin_mode::out);
    g.write(chip_select, tos::digital::high);

    g.set_pin_mode(data_command, tos::pin_mode::out);
    g.write(data_command, tos::digital::high);

    auto timer = tos::open(tos::devs::timer<2>);
    auto alarm = tos::open(tos::devs::alarm, timer);

    tos::stm32::spi spi(tos::stm32::detail::spis[0], 19_pin, std::nullopt, 21_pin);

    g.set_pin_mode(5_pin, tos::pin_mode::out);

    tos::gde021a1 display(&spi, chip_select, data_command, 18_pin, 8_pin, delay);

    static tos::gfx::fixed_canvas<72, 172> frame_buffer;
    frame_buffer.fill(true);

    static constexpr auto font =
        tos::gfx::basic_font().mirror_horizontal().rotate_90_cw().inverted();

    draw_text_line(frame_buffer,
                   font,
                   "tos",
                   tos::gfx::point{0, 0},
                   tos::gfx::text_direction::vertical);

    using namespace tos;
    display.set_display_window(gfx::point{0, 0}, gfx::dimensions{172, 72});
    display.draw_framebuffer(gfx::dimensions{172, 72}, frame_buffer.data());
    display.refresh_display();

    tos::this_thread::block_forever();
}

void tos_main() {
    tos::launch(tos::stack_size_t{2048}, blink_task);
}
