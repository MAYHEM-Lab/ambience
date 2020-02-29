#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <tos/ft.hpp>

extern void tos_main();

extern "C" {
extern void (*start_ctors[])();
extern void (*end_ctors[])();
extern uint8_t __bss_start;
extern uint8_t __bss_end;
}

extern "C" void kernel_main() {
    // std::fill_n(&__bss_start, &__bss_end, 0);
    // std::for_each(start_ctors, end_ctors, [](auto ctor) { ctor(); });

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