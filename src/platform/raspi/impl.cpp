#include <algorithm>
#include <tos/interrupt.hpp>
#include <tos/scheduler.hpp>

extern void tos_main();

extern "C" {
extern void (*start_ctors[])();
extern void (*end_ctors[])();
extern uint8_t __bss_start;
extern uint8_t __bss_end;
}

extern "C" {
void* __dso_handle;
}

void mmu_init();

extern "C" void kernel_main() {
    mmu_init();

    std::for_each(start_ctors, end_ctors, [](auto ctor) {
        ctor();
    });

    tos::kern::enable_interrupts();

    tos_main();

    while (true) {
        auto res = tos::global::sched.schedule();
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