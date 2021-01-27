#include <tos/debug/dynamic_log.hpp>
#include <tos/debug/sinks/serial_sink.hpp>
#include <tos/ft.hpp>
#include <tos/peripheral/uart_16550.hpp>
#include <tos/peripheral/vga_text.hpp>

void thread() {
    auto uart_res = tos::x86_64::uart_16550::open();
    if (!uart_res) {
        tos::debug::panic("Could not open the uart");
    }
    auto& uart = force_get(uart_res);

    tos::x86_64::text_vga vga;
    vga.clear();
    tos::println(vga, "Hello amd64 Tos!");

    tos::debug::serial_sink uart_sink(&uart);
    tos::debug::detail::any_logger uart_log{&uart_sink};
    uart_log.set_log_level(tos::debug::log_level::log);
    tos::debug::set_default_log(&uart_log);

    LOG("Hello world!");

    while (true) {
        tos::this_thread::yield();
    }
}

tos::stack_storage store;
void tos_main() {
    tos::launch(store, thread);
}