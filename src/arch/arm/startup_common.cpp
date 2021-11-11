#include <algorithm>
#include <tos/compiler.hpp>
#include <tos/memory.hpp>
#include <tos/arm/startup_common.hpp>

extern "C" {
extern uint64_t _sidata;
extern void (*start_ctors[])(void);
extern void (*end_ctors[])(void);
}

namespace {
void copy_initialized_memory() {
    const auto data = tos::default_segments::data();
    auto data_start = reinterpret_cast<uint64_t*>(data.base.direct_mapped());

    // Copy initialized data
    std::copy_n(&_sidata, data.size, data_start);
}

void zero_out_bss() {
    auto bss = tos::default_segments::bss();
    auto bss_start = reinterpret_cast<uint64_t*>(bss.base.direct_mapped());
    auto bss_end = reinterpret_cast<uint64_t*>(bss.end().direct_mapped());

    // Zero out BSS
    std::fill(bss_start, bss_end, 0);
}

void call_global_ctors() {
    std::for_each(start_ctors, end_ctors, [](auto x) { x(); });
}
} // namespace

namespace tos {
void boot_initialize() {
    copy_initialized_memory();
    zero_out_bss();
    call_global_ctors();
}
} // namespace tos

extern "C" {
void PendSV_Handler() {
    while (true)
        ;
}
void NMI_Handler() {
    while (true)
        ;
}
void DebugMon_Handler() {
    while (true)
        ;
}

[[noreturn]] int main();
void SystemInit();
void Reset_Handler() {
    tos::boot_initialize();
    SystemInit();
    main();
    TOS_UNREACHABLE();
}
}