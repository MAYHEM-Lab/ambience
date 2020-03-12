#include <arch/drivers.hpp>
#include <asm.hpp>
#include <tos/ft.hpp>
#include <tos/print.hpp>

extern "C" {
void tos_force_reset() {
    tos::aarch64::intrin::bkpt();
}
}

tos::stack_storage<TOS_DEFAULT_STACK_SIZE> stack;
void tos_main() {
    tos::launch(stack, [] {
        tos::raspi3::uart0 uart;
        tos::println(uart, "Hello from tos!");

        uint32_t el;
        asm volatile("mrs %0, CurrentEL" : "=r"(el));
        tos::println(uart, "Execution Level:", int((el >> 2) & 3));

        tos::raspi3::framebuffer fb({1024, 768});
        tos::println(uart, fb.dims().width, fb.dims().height);
        tos::println(uart, fb.get_buffer().data(), fb.get_buffer().size_bytes());

        for (auto& c : fb.get_buffer()) {
            c = 255;
        }

        tos::println(uart, "Filled");
        while (true) {
            uint8_t c;
            uart->write(uart->read(tos::monospan(c)));
        }
    });
}