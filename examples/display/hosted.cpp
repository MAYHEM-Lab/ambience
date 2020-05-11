#include <arch/display.hpp>
#include <arch/drivers.hpp>
#include <tos/debug/dynamic_log.hpp>
#include <tos/debug/sinks/clock_adapter.hpp>
#include <tos/debug/sinks/serial_sink.hpp>

void display_task() {
    tos::x86::display d({296, 128});
    auto painter = d.get_painter();
    painter->draw(tos::gfx::rectangle{{0, 0}, {128, 64}},
                  tos::gfx::fixed_color{{0, 0, 128}});
    painter->draw(tos::gfx::line{{0, 0}, {50, 50}}, tos::gfx::fixed_color{{0, 128, 0}});
    tos::this_thread::block_forever();
}

void tos_main() {
    tos::debug::set_default_log(
        new tos::debug::detail::any_logger(new tos::debug::clock_sink_adapter{
            tos::debug::serial_sink(tos::x86::stderr_adapter{}),
            tos::x86::clock<std::chrono::system_clock>{}}));

    tos::launch(tos::alloc_stack, display_task);
}