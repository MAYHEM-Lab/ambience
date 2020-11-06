#include "tos/thread.hpp"
#include <arch/drivers.hpp>
#include <common/clock.hpp>
#include <deque>
#include <tos/debug/dynamic_log.hpp>
#include <tos/debug/log.hpp>
#include <tos/debug/sinks/serial_sink.hpp>
#include <tos/ft.hpp>
#include <tos/gfx/bitmap_io.hpp>
#include <tos/gfx/color.hpp>
#include <tos/gfx/fonts/firacode_regular.hpp>
#include <tos/gfx/fonts/notosans_regular.hpp>
#include <tos/gfx/fonts/opensans_regular.hpp>
#include <tos/gfx/fonts/ubuntu_regular.hpp>
#include <tos/gfx/truetype.hpp>
#include <tos/memory/bump.hpp>
#include <tos/periph/bcm2837_clock.hpp>
#include <tos/print.hpp>

template<class T>
void clear(tos::span<T> buf) {
    std::memset(buf.data(), 0, buf.size());
}

namespace tos {
template<class FramebufferT>
class terminal : public self_pointing<terminal<FramebufferT>> {
public:
    explicit terminal(FramebufferT fb, gfx::font&& font)
        : m_fb{std::move(fb)}
        , m_line_height{24}
        , m_max_col{m_fb->dims().width / 16}
        , m_max_rows{m_fb->dims().height / m_line_height}
        , m_font{std::move(font)} {
        m_lines.emplace_back();
        m_line_buf.resize((m_line_height + 1) * m_fb->dims().width);
    }

    int write(span<const uint8_t> buf) {
        for (auto c : buf) {
            switch (c) {
            case '\r':
                caret_return();
                break;
            case '\n':
                new_line();
                break;
            case 0x7F:
            case 0x08:
                // backspace
                if (m_cur_col == 0) {
                    break;
                }
                m_cur_col--;
                process_char(' ');
                m_cur_col--;
                break;
            default:
                process_char(c);
                break;
            }
        }
        render();
        return buf.size();
    }

private:
    void render_line(std::string_view line, int line_number) {
        auto buf = m_fb->get_buffer();
        tos::gfx::basic_bitmap_view<tos::gfx::rgb8> fbview(
            {reinterpret_cast<tos::gfx::rgb8*>(buf.data()), buf.size() / 3},
            m_fb->get_pitch() / 3,
            m_fb->dims());

        tos::gfx::basic_bitmap_view<tos::gfx::mono8> view(
            m_line_buf, fbview.stride(), {fbview.dims().width, m_line_height + 1});
        clear(view.raw_data());
        auto res = m_font.render_text(line, m_line_height, view);

        fbview =
            fbview.slice({0, int16_t(m_line_height * line_number)},
                         {fbview.dims().width,
                          fbview.dims().height - int16_t(m_line_height * line_number)});

        for (int i = 0; i < res.dims().height; ++i) {
            for (int j = 0; j < res.dims().width; ++j) {
                auto col = res.at(i, j);
                fbview.at(i, j) = tos::gfx::rgb8{col.bw, col.bw, col.bw};
            }
        }
    }

    void render() {
        if (m_screen_dirty || m_line_dirty) {
            clear(m_fb->get_buffer());
            // redraw everything
            for (uint32_t i = 0; i < m_lines.size(); ++i) {
                render_line(m_lines[i], i);
            }
            m_screen_dirty = false;
            m_line_dirty = false;
            m_fb->swap_buffers();
        } else if (m_line_dirty) {
            // redraw line
            render_line(m_lines[m_cur_row], m_cur_row);
            m_line_dirty = false;
        }
    }

    void caret_return() {
        m_cur_col = 0;
    }

    void new_line() {
        m_lines.emplace_back();
        m_line_dirty = true;
        if (m_lines.size() > static_cast<size_t>(m_max_rows)) {
            m_lines.pop_front();
            m_screen_dirty = true;
            return;
        }
        ++m_cur_row;
    }

    void process_char(uint8_t c) {
        if (static_cast<size_t>(m_cur_col) == m_lines[m_cur_row].size()) {
            m_lines[m_cur_row].push_back(c);
        } else {
            m_lines[m_cur_row][m_cur_col] = c;
        }
        ++m_cur_col;
        m_line_dirty = true;
    }

    FramebufferT m_fb;

    int32_t m_line_height = 24;

    bool m_line_dirty = false;
    bool m_screen_dirty = false;

    std::deque<std::string> m_lines;

    int32_t m_cur_col = 0;
    int32_t m_max_col = 80;

    int32_t m_max_rows = 40;
    int32_t m_cur_row = 0;

    gfx::font m_font;
    std::vector<gfx::mono8> m_line_buf;
};
} // namespace tos

namespace debug = tos::debug;
void raspi_main() {
    auto uart = tos::open(tos::devs::usart<0>, tos::uart::default_115200);
    tos::println(uart, "Hello from tos!");
    auto serial = tos::raspi3::get_board_serial();
    tos::println(uart, "Serial no:", serial);
    tos::debug::serial_sink uart_sink(&uart);
    tos::debug::detail::any_logger uart_log{&uart_sink};
    tos::debug::set_default_log(&uart_log);

    debug::log("Log init complete");

    uint32_t el;
    asm volatile("mrs %0, CurrentEL" : "=r"(el));
    debug::log("Execution Level:", int((el >> 2) & 3));

    tos::periph::clock_manager clock_man;
    debug::log("CPU Freq:", clock_man.get_frequency(tos::bcm283x::clocks::arm));
    debug::log("Max CPU Freq:", clock_man.get_max_frequency(tos::bcm283x::clocks::arm));
    clock_man.set_frequency(tos::bcm283x::clocks::arm,
                            clock_man.get_max_frequency(tos::bcm283x::clocks::arm));
    debug::log("CPU Freq:", clock_man.get_frequency(tos::bcm283x::clocks::arm));

    tos::raspi3::framebuffer fb({1280, 720});

    tos::raspi3::interrupt_controller ic;
    tos::raspi3::system_timer timer(ic);
    tos::clock clock(&timer);

    auto now = clock.now();
    clear(fb.get_buffer());
    auto diff = clock.now() - now;
    tos::debug::info("Cleared screen in", diff);

    // tos::alarm alarm(timer);
    tos::terminal term(&fb, tos::gfx::font::make(tos::gfx::fonts::firacode_regular));
    tos::println(term, "Hello from tos");
    tos::println(term, "Raspberry pi serial no:", serial);
    tos::launch(tos::alloc_stack, [&] {
        while (true) {
            using namespace std::chrono_literals;

            tos::delay(clock, 1s, true);
            // tos::this_thread::sleep_for(alarm, 1s);
            tos::println(term, "Tick", timer.get_counter());
            tos::println(uart, timer.get_counter());
        }
    });

    while (true) {
        uint8_t c;
        auto buf = uart->read(tos::monospan(c));
        uart->write(buf);
        term->write(buf);
    }

    tos::this_thread::block_forever();
}

tos::stack_storage<TOS_DEFAULT_STACK_SIZE> stack;
void tos_main() {
    tos::launch(stack, raspi_main);
}
