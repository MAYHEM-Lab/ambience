#include <algorithm>
#include <tos/interrupt.hpp>
#include <tos/scheduler.hpp>
#include <tos/aarch64/assembly.hpp>

extern void tos_main();

extern "C" {
extern void (*start_ctors[])();
extern void (*end_ctors[])();
}

extern "C" {
void* __dso_handle;
}

void mmu_init();

extern "C" {
[[gnu::used]]
void kernel_main() {
    mmu_init();

    std::for_each(start_ctors, end_ctors, [](auto ctor) { ctor(); });

    tos::kern::enable_interrupts();

    tos_main();

    while (true) {
        auto res = tos::global::sched.schedule();
        if (res == tos::exit_reason::restart) {
        }

        if (res == tos::exit_reason::power_down) {
            tos::aarch64::wfi();
        }
        if (res == tos::exit_reason::idle) {
            tos::aarch64::wfi();
        }
        if (res == tos::exit_reason::yield) {
        }
    }
}
}