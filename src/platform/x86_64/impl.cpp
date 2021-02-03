#include <algorithm>
#include <cmath>
#include <string_view>
#include <tos/memory.hpp>
#include <tos/multiboot.hpp>
#include <tos/print.hpp>
#include <tos/scheduler.hpp>
#include <tos/self_pointing.hpp>
#include <tos/span.hpp>
#include <tos/stack_storage.hpp>
#include <tos/x86_64/assembly.hpp>
#include <tos/x86_64/gdt.hpp>
#include <tos/x86_64/idt.hpp>
#include <tos/x86_64/mmu.hpp>
#include <tos/x86_64/pic.hpp>
#include <tos/x86_64/port.hpp>

extern void tos_main();

namespace {
using namespace tos::x86_64;

auto idt = default_idt();

struct [[gnu::packed]] {
    uint16_t limits;
    uint64_t base;
} idt_thing;

tos::expected<void, idt_error> idt_setup() {
    idt_thing.base = reinterpret_cast<uint64_t>(&idt);
    idt_thing.limits = sizeof idt - 1;
    asm volatile("lidt %0" : : "m"(idt_thing));
    return {};
}

[[gnu::section(".nozero")]] tos::stack_storage<4096 * 8> main_stack{};
} // namespace

namespace {
NO_INLINE
void enable_fpu() {
    using namespace tos::x86_64;

    auto cr0 = read_cr0();
    cr0 |= (1 << 1);
    cr0 &= ~(1 << 2);
    cr0 &= ~(1 << 3); // Task switched, without this, we would get an exception at first
    // the first FPU usage
    cr0 |= (1 << 5);
    write_cr0(cr0);

    asm volatile("finit");
}

NO_INLINE
void enable_sse() {
    /*
        clear the CR0.EM bit (bit 2) [ CR0 &= ~(1 << 2) ]
        set the CR0.MP bit (bit 1) [ CR0 |= (1 << 1) ]
        set the CR4.OSFXSR bit (bit 9) [ CR4 |= (1 << 9) ]
        set the CR4.OSXMMEXCPT bit (bit 10) [ CR4 |= (1 << 10) ]
     */
    using namespace tos::x86_64;

    auto cr4 = read_cr4();
    cr4 |= (1 << 9) | (1 << 10);
    write_cr4(cr4);
}

[[gnu::section(".nozero")]] gdt_entry gdt_entry_data[3];

[[gnu::section(".nozero")]] struct [[gnu::packed]] {
    uint16_t sz;
    uint64_t ptr;
} gdt;

[[gnu::noinline]] void setup_gdt() {
    memset(&gdt_entry_data, 0, sizeof(gdt_entry_data));

    gdt_entry_data[0] = {.limit_low = 0xffff,
                         .base_low = 0,
                         .base_mid = 0,
                         .access = 0,
                         .opts_limit_mid = 1,
                         .base_hi = 0};

    gdt_entry_data[1] = {.limit_low = 0,
                         .base_low = 0,
                         .base_mid = 0,
                         .access = 0x9a,
                         .opts_limit_mid = 0b10101111,
                         .base_hi = 0};

    gdt_entry_data[2] = {.limit_low = 0,
                         .base_low = 0,
                         .base_mid = 0,
                         .access = 0x92,
                         .opts_limit_mid = 0,
                         .base_hi = 0};

    gdt.sz = sizeof gdt_entry_data - 1;
    gdt.ptr = reinterpret_cast<uint64_t>(&gdt_entry_data);
    asm volatile("lgdt %0" : : "m"(gdt));
}

extern "C" {
[[gnu::section(".nozero")]] alignas(
    4096) std::array<tos::x86_64::table_entry, 512> p4_table;
[[gnu::section(".nozero")]] alignas(
    4096) std::array<tos::x86_64::table_entry, 512> p3_table;
[[gnu::section(".nozero")]] translation_table p2_tables[1];
[[gnu::section(".nozero")]] translation_table p1_tables[3];
}

extern "C" {
void set_up_page_tables() {
    memset(&p4_table, 0, sizeof p4_table);
    memset(&p3_table, 0, sizeof p3_table);

    p4_table[0].zero().valid(true).writeable(true).page_num(
        reinterpret_cast<uintptr_t>(&p3_table));

    p3_table[0].zero().valid(true).writeable(true).page_num(
        reinterpret_cast<uintptr_t>(&p2_tables[0]));

    for (int i = 0; i < std::size(p1_tables); ++i) {
        p2_tables[0][i]
            .zero()
            .page_num(reinterpret_cast<uintptr_t>(&p1_tables[i]))
            .valid(true)
            .writeable(true);
    }

    static constexpr tos::memory_range io_regions[] = {
        {0x00000000, 0x000003FF - 0x00000000},
        {0x00000400, 0x000004FF - 0x00000400},
        {0x00080000, 0x0009FFFF - 0x00080000},
        {0x000A0000, 0x000BFFFF - 0x000A0000},
        {0x000C0000, 0x000C7FFF - 0x000C0000},
        {0x000C8000, 0x000EFFFF - 0x000C8000},
        {0x000F0000, 0x000FFFFF - 0x000F0000},
        {0x00F00000, 0x00FFFFFF - 0x00F00000},
        {0xC0000000, 0xFFFFFFFF - 0xC0000000},
    };

    auto is_mapped = [](const tos::memory_range& region) -> bool {
        for (auto& io_reg : io_regions) {
            if (tos::intersection(region, io_reg)) {
                return true;
            }
        }
        return tos::intersection(region, tos::default_segments::image()).has_value();
    };

    for (int i = 0; i < 2; ++i) {
        auto& table = p1_tables[i];
        for (int j = 0; j < 512; ++j) {
            auto page_range = tos::memory_range{uintptr_t((i * 512 + j) << 12), 4096};

            if (!is_mapped(page_range)) {
                continue;
            }

            table[j].zero().valid(true).writeable(true).page_num((i * 512 + j) << 12);
        }
    }

    tos::x86_64::write_cr3(reinterpret_cast<uint64_t>(&p4_table));
}
}
} // namespace

extern "C" {
extern void (*start_ctors[])(void);
extern void (*end_ctors[])(void);

[[noreturn]] void _post_start(const tos::multiboot::info_t* info);
[[noreturn]] void _prestart(const tos::multiboot::info_t* info);

[[noreturn]] [[gnu::section(".text.entry")]] void
_start(const tos::multiboot::info_t* info) {
    set_stack_ptr(reinterpret_cast<char*>(&main_stack) + sizeof main_stack);
    _prestart(info);
    TOS_UNREACHABLE();
}

[[noreturn]] void _prestart([[maybe_unused]] const tos::multiboot::info_t* info) {
    setup_gdt();
    asm volatile("mov %0, %%ds" : : "r"(0x10));
    asm volatile("mov %0, %%es" : : "r"(0x10));
    asm volatile("mov %0, %%fs" : : "r"(0x10));
    asm volatile("mov %0, %%gs" : : "r"(0x10));
    asm volatile("mov %0, %%ss" : : "r"(0x10));
    asm volatile("push $0x8\n"
                 "push $_post_start\n"
                 "lretq");
    TOS_UNREACHABLE();
}

[[noreturn]] [[gnu::used]] void
_post_start([[maybe_unused]] const tos::multiboot::info_t* info) {
    enable_fpu();

    enable_sse();

    set_up_page_tables();

    auto bss = tos::default_segments::bss();
    auto bss_start = reinterpret_cast<uint64_t*>(bss.base);
    auto bss_end = reinterpret_cast<uint64_t*>(bss.end());

    // Zero out BSS
    std::fill(bss_start, bss_end, 0);

    // Call constructors
    std::for_each(start_ctors, end_ctors, [](auto x) { x(); });

    idt_setup();

    pic::initialize();

    tos::kern::enable_interrupts();

    tos_main();

    while (true) {
        {
            tos::int_guard ig;

            auto res = tos::global::sched.schedule(ig);
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
}
}