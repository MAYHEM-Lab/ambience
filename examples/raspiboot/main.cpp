#include <arch/drivers.hpp>
#include <common/clock.hpp>
#include <deque>
#include <tos/debug/log.hpp>
#include <tos/ft.hpp>
#include <tos/gfx/text.hpp>
#include <tos/memory/bump.hpp>
#include <tos/print.hpp>

auto font = tos::gfx::basic_font().mirror_horizontal().resize<16, 16>();

void clear(tos::span<uint8_t> buf) {
    std::memset(buf.data(), 0, buf.size());
}

namespace tos {
template<class FramebufferT>
class terminal : public self_pointing<terminal<FramebufferT>> {
public:
    explicit terminal(FramebufferT fb)
        : m_fb{std::move(fb)}
        , m_max_rows{m_fb->dims().height / 20}
        , m_max_col{m_fb->dims().width / 16} {
        m_lines.emplace_back();
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
    tos::gfx::point line_pos(int line) {
        return {0, static_cast<uint16_t>(line * 20)};
    }

    void render_line(std::string_view line, int line_number) {
        tos::gfx::draw_text_line(m_fb, font, line, line_pos(line_number));
    }

    void render() {
        if (m_screen_dirty || m_line_dirty) {
            clear(m_fb->get_buffer());
            // redraw everything
            for (int i = 0; i < m_lines.size(); ++i) {
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
        if (m_lines.size() > m_max_rows) {
            m_lines.pop_front();
            m_screen_dirty = true;
            return;
        }
        ++m_cur_row;
    }

    void process_char(uint8_t c) {
        if (m_cur_col == m_lines[m_cur_row].size()) {
            m_lines[m_cur_row].push_back(c);
        } else {
            m_lines[m_cur_row][m_cur_col] = c;
        }
        ++m_cur_col;
        m_line_dirty = true;
    }

    FramebufferT m_fb;

    bool m_line_dirty = false;
    bool m_screen_dirty = false;

    std::deque<std::string> m_lines;

    int m_cur_col = 0;
    int m_max_col = 80;

    int m_cur_row = 0;
    int m_max_rows = 40;
};
} // namespace tos

namespace tos::raspi3 {
enum class clocks
{
    reserved,
    emmc,
    uart,
    arm,
    core,
    v3d,
    h264,
    isp,
    sdram,
    pixel,
    pwm,
    hevc,
    emmc2,
    m2mc,
    pixel_bvb
};

enum class clock_tags
{
    get_clock_rate = 0x00030002,
    get_max_clock_rate = 0x00030004,
    set_clock_rate = 0x00038002
};

class clock_manager {
public:
    uint32_t get_max_frequency(clocks clock) {
        property_channel props;

        property_channel_tags_builder builder;
        auto buf = builder
                       .add(static_cast<uint32_t>(clock_tags::get_max_clock_rate),
                            {static_cast<uint32_t>(clock), 0})
                       .end();

        auto res = props.transaction(buf);
        if (!res) {
            // noo
            tos::debug::panic("can't initialize framebuffer");
        }

        if (buf[1] == 0) {
            tos::debug::panic("bad response");
        }

        auto code = buf[1] - 0x80000000;
        if (code != 0) {
            tos::debug::panic("error");
        }

        return buf[6];
    }

    uint32_t get_frequency(clocks clock) {
        property_channel props;

        property_channel_tags_builder builder;
        auto buf = builder
                       .add(static_cast<uint32_t>(clock_tags::get_clock_rate),
                            {static_cast<uint32_t>(clock), 0})
                       .end();

        auto res = props.transaction(buf);
        if (!res) {
            // noo
            tos::debug::panic("can't initialize framebuffer");
        }

        if (buf[1] == 0) {
            tos::debug::panic("bad response");
        }

        auto code = buf[1] - 0x80000000;
        if (code != 0) {
            tos::debug::panic("error");
        }

        return buf[6];
    }

    void set_frequency(clocks clock, uint32_t hertz) {
        property_channel props;

        property_channel_tags_builder builder;
        auto buf = builder
                       .add(static_cast<uint32_t>(clock_tags::set_clock_rate),
                            {static_cast<uint32_t>(clock), hertz, 0})
                       .end();

        auto res = props.transaction(buf);
        if (!res) {
            // noo
            tos::debug::panic("can't initialize framebuffer");
        }

        if (buf[1] == 0) {
            tos::debug::panic("bad response");
        }

        auto code = buf[1] - 0x80000000;
        if (code != 0) {
            tos::debug::panic("error");
        }
    }

private:
};
} // namespace tos::raspi3

tos::stack_storage<TOS_DEFAULT_STACK_SIZE> stack;
void tos_main() {
    tos::launch(stack, [] {
        tos::raspi3::uart0 uart;
        tos::println(uart, "Hello from tos!");
        auto serial = tos::raspi3::get_board_serial();
        tos::println(uart, "Serial no:", serial);

        uint32_t el;
        asm volatile("mrs %0, CurrentEL" : "=r"(el));
        tos::println(uart, "Execution Level:", int((el >> 2) & 3));

        tos::raspi3::clock_manager clock_man;
        tos::println(
            uart, "CPU Freq:", clock_man.get_frequency(tos::raspi3::clocks::arm));
        tos::println(
            uart, "Max CPU Freq:", clock_man.get_max_frequency(tos::raspi3::clocks::arm));
        clock_man.set_frequency(tos::raspi3::clocks::arm,
                                clock_man.get_max_frequency(tos::raspi3::clocks::arm));
        tos::println(
            uart, "CPU Freq:", clock_man.get_frequency(tos::raspi3::clocks::arm));


        tos::raspi3::framebuffer fb({1920, 1080});
        tos::println(uart, fb.virtual_dims().width, fb.virtual_dims().height);
        tos::println(uart, fb.dims().width, fb.dims().height);
        tos::println(uart,
                     fb.get_buffer().data(),
                     fb.get_buffer().size_bytes(),
                     fb.get_pitch(),
                     fb.is_rgb(),
                     fb.bit_depth());

        tos::raspi3::interrupt_controller ic;
        tos::raspi3::system_timer timer(ic);
        tos::clock clock(&timer);

        auto now = clock.now();
        clear(fb.get_buffer());
        auto diff = clock.now() - now;
        tos::println(uart, "Cleared screen in", diff);

        // tos::alarm alarm(timer);
        tos::terminal term(&fb);
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
    });
}
