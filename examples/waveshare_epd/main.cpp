#include <arch/drivers.hpp>
#include <common/epd/waveshare/bw29.hpp>
#include <tos/arm/assembly.hpp>
#include <tos/ft.hpp>

#undef SCB
#undef DWT

#include <queue>
#include <tos/arm/core.hpp>
#include <tos/debug/dynamic_log.hpp>
#include <tos/debug/sinks/serial_sink.hpp>
#include <tos/gfx2/bit_painter.hpp>
#include <tos/gui/decorators.hpp>
#include <tos/gui/elements.hpp>

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

    tos::debug::serial_sink sink(&link);
    tos::debug::detail::any_logger logger(&sink);
    logger.set_log_level(tos::debug::log_level::log);
    tos::debug::set_default_log(&logger);

    tos::this_thread::block_forever();
}

class epd_ui {
public:
    void draw(tos::services::painter& painter, const tos::gfx2::rectangle& at) {
        using namespace tos::gui::elements;
        using namespace tos::gui::decorators;

        bordered{box{}}.draw(painter, at);

        padding{horizontal_ruler{}, 10, 10, 0, 0}.draw(
            painter,
            {{0, at.dims().height() / 2}, {at.dims().width(), at.dims().height() / 2}});

        rbox{{}, 10}.draw(painter, {{10, 10}, {40, 30}});

        align_center_middle{round_bordered{4, padding{label("hello"), 4, 4, 4, 4}}}.draw(
            painter, {at.corner(), {at.dims().width(), at.dims().height() / 2}});
    }
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

    auto painter = tos::gfx2::bit_painter{fb, {sz.width, sz.height}};

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