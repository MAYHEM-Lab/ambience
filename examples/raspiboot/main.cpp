#include <arch/drivers.hpp>
#include <tos/ft.hpp>
#include <tos/print.hpp>

tos::stack_storage<TOS_DEFAULT_STACK_SIZE> stack;
void tos_main() {
    tos::launch(stack, [] {
        tos::raspi3::uart0 uart;
        tos::println(uart, "Hello from tos!");

        while (true) {
            uint8_t c;
            uart->write(uart->read(tos::monospan(c)));
        }
    });
}