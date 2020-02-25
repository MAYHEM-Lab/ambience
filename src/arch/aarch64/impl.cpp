#include <cstddef>
#include <cstdint>
#include <tos/ft.hpp>

extern void tos_main();

extern "C"
void kernel_main(uint32_t r0, uint32_t r1, uint32_t atags) {
    tos_main();

    while (true) {
        auto res = tos::sched.schedule();
        if (res == tos::exit_reason::restart) {
        }

        if (res == tos::exit_reason::power_down) {
        }
        if (res == tos::exit_reason::idle) {
        }
        if (res == tos::exit_reason::yield) {
        }
    }
}