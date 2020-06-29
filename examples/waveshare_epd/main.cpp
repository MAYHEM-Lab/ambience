#include <arch/drivers.hpp>
#include <common/epd/waveshare/bw29.hpp>
#include <tos/arm/assembly.hpp>
#include <tos/ft.hpp>
#undef SCB
#undef DWT
#include <gfx_generated.hpp>
#include <painter_generated.hpp>
#include <queue>
#include <tos/arm/core.hpp>
#include <tos/debug/dynamic_log.hpp>
#include <tos/debug/sinks/serial_sink.hpp>
#include <tos/gfx/text.hpp>

struct cyccnt_clock {
public:
    cyccnt_clock() {
        // Enable CoreDebug_DEMCR_TRCENA_Msk
        tos::arm::SCB::DEMCR.write(tos::arm::SCB::DEMCR.read() | 0x01000000);

        tos::arm::DWT::CYCCNT.write(0);
        tos::arm::DWT::CONTROL.write(tos::arm::DWT::CONTROL.read() | 1);
    }

    typedef std::ratio<1, 64'000'000> clock_cycle;

    using rep = uint64_t;
    using period = clock_cycle;
    using duration = std::chrono::duration<rep, period>;
    using time_point = std::chrono::time_point<cyccnt_clock>;

    [[nodiscard]] time_point now() const {
        return time_point(duration(tos::arm::DWT::CYCCNT.read()));
    }

private:
};

namespace tos::gfx2 {
point other_corner(const rectangle& rect) {
    return {rect.corner().x() + rect.dims().width(),
            rect.corner().y() + rect.dims().height()};
}

std::array<line, 4> lines(const rectangle& rect) {
    auto a1 = rect.corner();
    auto a2 = point{rect.corner().x(), int16_t(rect.corner().y() + rect.dims().height())};
    auto a3 = other_corner(rect);
    auto a4 = point{int16_t(rect.corner().x() + rect.dims().width()), rect.corner().y()};
    return {line{a1, a2}, line{a2, a3}, {a3, a4}, {a4, a1}};
}
} // namespace tos::gfx2

namespace {
using tos::expected;
using errors = mpark::variant<mpark::monostate>;

void comm_thread() {
    using namespace tos::tos_literals;
#ifdef TOS_PLATFORM_stm32_hal
    auto link = tos::open(tos::devs::usart<1>, tos::uart::default_115200, 23_pin, 22_pin);
#elif defined(TOS_PLATFORM_nrf52)
    auto link = open(tos::devs::usart<0>, tos::uart::default_115200, 8_pin, 6_pin);
#endif
    //    auto transport = tos::io::serial_packets{&link};

    //    remote_log log(transport.get_channel(4));
    //    tos::debug::lidl_sink sink(log);
    tos::debug::serial_sink sink(&link);
    tos::debug::detail::any_logger logger(&sink);
    logger.set_log_level(tos::debug::log_level::log);
    tos::debug::set_default_log(&logger);

    tos::this_thread::block_forever();
}

class bit_painter : public tos::services::painter {
public:
    bit_painter(tos::span<uint8_t> buffer, tos::gfx2::size dims)
        : m_fb{buffer}
        , m_dims{dims}
        , m_style(false, tos::gfx2::binary_color{true}) {
    }

    void draw_line(const tos::gfx2::point& p0, const tos::gfx2::point& p1) {
        int dx = p1.x() - p0.x();
        int dy = p1.y() - p0.y();

        int dLong = abs(dx);
        int dShort = abs(dy);

        int offsetLong = dx > 0 ? 1 : -1;
        int offsetShort = dy > 0 ? m_dims.width() : -m_dims.width();

        if (dLong < dShort) {
            using std::swap;
            swap(dShort, dLong);
            swap(offsetShort, offsetLong);
        }

        int error = dLong / 2;
        int index = p0.y() * m_dims.width() + p0.x();
        const int offset[] = {offsetLong, offsetLong + offsetShort};
        const int abs_d[] = {dShort, dShort - dLong};
        for (int i = 0; i <= dLong; ++i) {
            draw(bit_location(index));
            const int errorIsTooBig = error >= dLong;
            index += offset[errorIsTooBig];
            error += abs_d[errorIsTooBig];
        }
    }

    int8_t draw_line(const tos::gfx2::line& l) override {
        auto& [p0, p1] = l;
        draw_line(p0, p1);
        return 0;
    }

    int8_t draw_point(const tos::gfx2::point& p) override {
        auto loc = bit_location(p);
        draw(loc);
        return 0;
    }

    int8_t draw_rect(const tos::gfx2::rectangle& rect) override {
        if (m_style.fill()) {
            if (rect.corner() == tos::gfx2::point{0, 0} &&
                rect.dims() == get_dimensions()) {
                fill();
                return 0;
            }
            for (int16_t row = rect.corner().y(); row < other_corner(rect).y(); ++row) {
                for (int16_t col = rect.corner().x(); col < other_corner(rect).x();
                     ++col) {
                    draw(tos::gfx::point{col, row});
                }
            }
        } else {
            for (auto& line : lines(rect)) {
                draw_line(line);
            }
        }
        return 0;
    }

    int8_t set_style(const tos::services::style& s) override {
        m_style = s;
        return 0;
    }

    tos::gfx2::size get_dimensions() override {
        return m_dims;
    }

    void fill() {
        auto fbyte = m_style.color().binary().col() ? 0xFF : 0;
        fill_byte(fbyte);
    }

    void fill_byte(uint8_t fbyte) {
        std::fill(m_fb.begin(), m_fb.end(), fbyte);
    }

    void draw(const tos::gfx::point& p) {
        auto loc = bit_location(p);
        draw(loc);
    }

    int8_t draw_text(std::string_view text, const tos::gfx2::point& p) override {
        static constexpr auto font = tos::gfx::basic_font().mirror_horizontal();
        LOG("Drawing text", text);
        draw_text_line(text, font, p);
        return 0;
    }

    void flush() {
    }

private:
    template<class FontT>
    void draw_text_line(std::string_view str, const FontT& font, tos::gfx2::point p) {
        LOG("Drawing text line", str);
        for (char c : str) {
            if (c == 0)
                return;
            auto ch = font.get(c);
            if (!ch) {
                return;
            }
            auto scaled = *ch;
            using tos::gfx::copy;

            for (size_t i = 0; i < scaled.height(); ++i) {
                for (size_t j = 0; j < scaled.width(); ++j) {
                    if (scaled.get_pixel(j, i)) {
                        draw_point({p.x() + j, p.y() + i});
                    }
                }
            }

            p.x() += scaled.width();
        }
    }

    struct bit_loc {
        int byte_num;
        int bit_pos;
    };

    uint8_t read_byte(int num) {
        return m_fb[num];
    }

    void write_byte(int num, uint8_t byte) {
        m_fb[num] = byte;
    }

    [[nodiscard]] bit_loc bit_location(const tos::gfx2::point& p) const {
        auto absolute_bit_pos = p.y() * m_dims.width() + p.x();
        return bit_location(absolute_bit_pos);
    }

    [[nodiscard]] bit_loc bit_location(const tos::gfx::point& p) const {
        auto absolute_bit_pos = p.y * m_dims.width() + p.x;
        return bit_location(absolute_bit_pos);
    }

    void draw(const bit_loc& loc) {
        auto byte = read_byte(loc.byte_num);
        if (m_style.color().binary().col()) {
            byte |= 1 << loc.bit_pos;
        } else {
            byte &= ~(1 << loc.bit_pos);
        }
        write_byte(loc.byte_num, byte);
    }

    [[nodiscard]] constexpr bit_loc bit_location(int bitpos) const {
        auto byte_num = bitpos / 8;
        auto bit_pos = bitpos % 8;
        return {byte_num, 7 - bit_pos};
    }

    tos::span<uint8_t> m_fb;
    tos::gfx2::size m_dims;
    tos::services::style m_style;
};

struct limits {};

class label {
public:
    explicit label(std::string str)
        : m_str{std::move(str)} {
    }

    tos::gfx2::size size() const {
        return {m_str.size() * 8, 8};
    }

    void draw(tos::services::painter& painter, const tos::gfx2::rectangle& at) {
        painter.draw_text(m_str, at.corner());
    }

private:
    std::string m_str;
};

struct box {
    void draw(tos::services::painter& painter, const tos::gfx2::rectangle& at) {
        tos::services::style fill_style(true, tos::gfx2::binary_color{true});
        painter.set_style(fill_style);
        painter.draw_rect(at);
    }
};

struct horizontal_ruler {
    void draw(tos::services::painter& painter, const tos::gfx2::rectangle& at) {
        tos::services::style fill_style(false, tos::gfx2::binary_color{false});
        painter.set_style(fill_style);
        painter.draw_line(
            {at.corner(), {at.corner().x() + at.dims().width(), at.corner().y()}});
    }
};

template<class Base>
struct padding {
    Base base;
    int left, right, top, bottom;

    auto size() {
        tos::gfx2::size base_size = base.size();
        return tos::gfx2::size{base_size.width() + left + right,
                               base_size.height() + top + bottom};
    }

    void draw(tos::services::painter& painter, const tos::gfx2::rectangle& at) {
        tos::gfx2::size base_size = at.dims();
        auto actual_size = tos::gfx2::size{base_size.width() - left - right,
                                           base_size.height() - top - bottom};
        base.draw(painter,
                  {{at.corner().x() + left, at.corner().y() + top}, actual_size});
    }
};

template<class Base>
padding(Base&& base, int left, int right, int top, int bottom) -> padding<Base>;

template<class Base>
struct bordered {
    Base base;

    auto size() {
        return base.size();
    }

    void draw(tos::services::painter& painter, const tos::gfx2::rectangle& at) {
        base.draw(painter, at);

        tos::services::style s(false, tos::gfx2::binary_color{false});
        painter.set_style(s);
        painter.draw_rect({at.corner(), {at.dims().width() - 1, at.dims().height() - 1}});
    }
};

template<class Base>
bordered(Base&& b) -> bordered<Base>;

template<class Base>
struct fixed_size {
    Base base;
    tos::gfx2::size sz;

    auto size() {
        return sz;
    }

    void draw(tos::services::painter& painter, const tos::gfx2::rectangle& at) {
        base.draw(painter, {at.corner(), sz});
    }
};

template<class Base>
fixed_size(Base&& b, const tos::gfx2::size&) -> fixed_size<Base>;

template<class Base>
struct align_center_middle {
    Base base;

    void draw(tos::services::painter& painter, const tos::gfx2::rectangle& at) {
        auto base_size = base.size();
        auto x_diff = (at.dims().width() - base_size.width()) / 2;
        auto y_diff = (at.dims().height() - base_size.height()) / 2;
        base.draw(painter,
                  {{at.corner().x() + x_diff, at.corner().y() + y_diff}, base_size});
    }
};

template<class Base>
align_center_middle(Base&& b) -> align_center_middle<Base>;

template<class... Layers>
struct view {
    view(Layers&&... layers)
        : m_layers{std::move(layers)...} {
    }

    void draw(tos::services::painter& painter, const tos::gfx2::rectangle& at) {
        std::apply([&](auto... x) { std::make_tuple(x.draw(painter, at)...); }, m_layers);
    }

    std::tuple<Layers...> m_layers;
};

class epd_ui {
public:
    void draw(tos::services::painter& painter, const tos::gfx2::rectangle& at) {
        //        painter.fill_byte(0xFF);
        //        painter.set_style(style{.color = false});
        bordered{box{}}.draw(painter, at);

        padding{horizontal_ruler{}, 10, 10, 0, 0}.draw(
            painter,
            {{0, at.dims().height() / 2}, {at.dims().width(), at.dims().height() / 2}});

        align_center_middle{bordered{padding{label("hello"), 4, 4, 4, 4}}}.draw(
            painter, {at.corner(), {at.dims().width(), at.dims().height() / 2}});
    }

private:
};

expected<void, errors> epd_main() {
    using namespace tos::tos_literals;

#ifdef TOS_PLATFORM_stm32_hal
    auto spi = tos::stm32::spi(tos::stm32::detail::spis[0], 5_pin, std::nullopt, 7_pin);
    auto busy_pin = 3_pin;   // D4
    auto reset_pin = 20_pin; // D5
    auto dc_pin = 17_pin;    // D6
    auto cs_pin = 4_pin;     // D7

    auto delay = [](std::chrono::microseconds us) {
        uint32_t end = (us.count() * (tos::stm32::ahb_clock / 1'000'000)) / 13.3;
        for (volatile int i = 0; i < end; ++i) {
            tos::arm::nop();
        }
    };
#elif defined(TOS_PLATFORM_nrf52)
    auto mosi_pin = 2_pin;
    auto clk_pin = 3_pin;
    auto busy_pin = 29_pin;
    auto reset_pin = 28_pin;
    auto dc_pin = 5_pin;
    auto cs_pin = 4_pin;
    auto spi = tos::nrf52::spi(clk_pin, std::nullopt, mosi_pin);

    auto delay = [](std::chrono::microseconds us) { nrfx_coredep_delay_us(us.count()); };
#endif

    cyccnt_clock clk;

    tos::waveshare_29bw epd(&spi, cs_pin, dc_pin, reset_pin, busy_pin, delay, true);

    auto first = clk.now();
    epd.clear_buffer(0xFF);
    epd.wait();
    auto diff = clk.now() - first;
    LOG("Clear took",
        std::chrono::duration_cast<std::chrono::milliseconds>(diff).count(),
        "ms");

    first = clk.now();
    epd.swap_buffers();
    diff = clk.now() - first;
    LOG("Swap took",
        std::chrono::duration_cast<std::chrono::milliseconds>(diff).count(),
        "ms");

    first = clk.now();
    epd.wait();
    diff = clk.now() - first;
    LOG("Wait took",
        std::chrono::duration_cast<std::chrono::milliseconds>(diff).count(),
        "ms");

    auto sz = epd.framebuffer_dims();
    auto fb = std::vector<uint8_t>(sz.area() / 8); // 1bpp

    auto painter = bit_painter{fb, {sz.width, sz.height}};

    epd_ui ui;
    first = clk.now();
    ui.draw(painter, {{0, 0}, painter.get_dimensions()});
    painter.flush();
    diff = clk.now() - first;
    LOG("Paint took",
        std::chrono::duration_cast<std::chrono::milliseconds>(diff).count(),
        "ms");

    epd.copy_buffer(fb, {{0, 0}, sz});
    epd.swap_buffers();
    epd.wait();

    tos::this_thread::block_forever();
    return {};
}

tos::stack_storage<1024> s;
tos::stack_storage<2048> epd_stack;
} // namespace

void tos_main() {
    tos::launch(s, comm_thread);
    tos::launch(epd_stack, [] { epd_main(); });
}