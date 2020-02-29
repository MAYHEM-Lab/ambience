#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <tos/ft.hpp>

extern void tos_main();

extern "C" {
extern void (*start_ctors[])(void);
extern void (*end_ctors[])(void);
extern uint32_t* __bss_start;
extern uint32_t __bss_size;
}

extern "C" void kernel_main([[maybe_unused]] uint32_t r0,
                            [[maybe_unused]] uint32_t r1,
                            [[maybe_unused]] uint32_t atags) {
    std::fill_n(__bss_start, __bss_size, 0);
    std::for_each(start_ctors, end_ctors, [](auto ctor) { ctor(); });

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