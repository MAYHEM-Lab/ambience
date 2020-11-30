#include <common/driver_base.hpp>
#include <cstring>
#include <deque>
#include <tos/gfx/bitmap_io.hpp>
#include <tos/gfx/color.hpp>
#include <tos/gfx/fonts/firacode_regular.hpp>
#include <tos/gfx/fonts/notosans_regular.hpp>
#include <tos/gfx/fonts/opensans_regular.hpp>
#include <tos/gfx/fonts/ubuntu_regular.hpp>
#include <tos/gfx/truetype.hpp>
#include <tos/span.hpp>
#include <arch/drivers.hpp>

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

void run_term() {
    tos::raspi3::framebuffer fb({1280, 720});

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
}