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
#include <tos/x86_64/syscall.hpp>
#include <tos/x86_64/tss.hpp>

extern "C" {
void __cxa_atexit(void (*)(void*), void*, void*) {
}
}

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

struct [[gnu::packed]] gdt_entries {
    gdt_entry normal[6];
    expanded_gdt_entry expanded[1];
};

[[gnu::section(".nozero")]] gdt_entries gdt_entry_data;

[[gnu::section(".nozero")]] struct [[gnu::packed]] {
    uint16_t sz;
    uint64_t ptr;
} gdt;

[[gnu::section(".nozero")]] tss tss_;

[[gnu::section(".nozero")]] tos::stack_storage<4096 * 8> interrupt_stack{};
void setup_tss(expanded_gdt_entry& entry) {
    static_assert(sizeof(tss_) == 104);
    entry.zero()
        .base(reinterpret_cast<uint64_t>(&tss_))
        .entry.type(0b1110'1001)
        .limit(sizeof(tss_))
        .long_mode(true);

    tss_.rsp[0] = reinterpret_cast<uintptr_t>(&interrupt_stack) + sizeof interrupt_stack;
    tss_.iopb_offset = sizeof tss_;
}

[[gnu::noinline]] void setup_gdt() {
    memset(&gdt_entry_data, 0, sizeof(gdt_entry_data));

    // null descriptor
    gdt_entry_data.normal[0].zero();

    // kernel code
    // Base, limit etc is ignored
    // type 0xa
    gdt_entry_data.normal[1] = {.limit_low = 0,
                                .base_low = 0,
                                .base_mid = 0,
                                .access = 0x9a, // 0b1001'1010
                                .opts_limit_mid = 0b1010'0000,
                                .base_hi = 0};

    // kernel data
    // type 0x2
    gdt_entry_data.normal[2] = {.limit_low = 0,
                                .base_low = 0,
                                .base_mid = 0,
                                .access = 0x92, // 0b1001'0010
                                .opts_limit_mid = 0,
                                .base_hi = 0};

    gdt_entry_data.normal[3].zero();

    // user data
    // type 0x2
    // DPL = 3 (ring 3)
    gdt_entry_data.normal[4] = {.limit_low = 0,
                                .base_low = 0,
                                .base_mid = 0,
                                .access = 0b1111'0010, // 0b1001'0010
                                .opts_limit_mid = 0,
                                .base_hi = 0};

    // user code
    // Base, limit etc is ignored
    // DPL = 3 (ring 3)
    gdt_entry_data.normal[5] = {.limit_low = 0,
                                .base_low = 0,
                                .base_mid = 0,
                                .access = 0b1111'1010, // 0b1001'1010
                                .opts_limit_mid = 0b1010'0000,
                                .base_hi = 0};

    setup_tss(gdt_entry_data.expanded[0]);

    gdt.sz = sizeof gdt_entry_data - 1;
    gdt.ptr = reinterpret_cast<uint64_t>(&gdt_entry_data);
    asm volatile("lgdt %0" : : "m"(gdt));

    static_assert(offsetof(gdt_entries, expanded) == 0x30);
    asm volatile("ltr %%ax" : : "a"(offsetof(gdt_entries, expanded) | 0x3ULL));
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

    p4_table[0]
        .zero()
        .valid(true)
        .writeable(true)
        .page_num(reinterpret_cast<uintptr_t>(&p3_table))
        .allow_user(true);

    p3_table[0]
        .zero()
        .valid(true)
        .writeable(true)
        .page_num(reinterpret_cast<uintptr_t>(&p2_tables[0]))
        .allow_user(true);

    for (size_t i = 0; i < std::size(p1_tables); ++i) {
        p2_tables[0][i]
            .zero()
            .page_num(reinterpret_cast<uintptr_t>(&p1_tables[i]))
            .valid(true)
            .writeable(true)
            .allow_user(true);
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

            table[j].zero().valid(true).page_num((i * 512 + j) << 12).allow_user(true);

            if (tos::intersection(tos::default_segments::text(), page_range)) {
                table[j].writeable(false).noexec(false);
            } else {
                table[j].noexec(true);
                if (tos::intersection(tos::default_segments::rodata(), page_range)) {
                    table[j].writeable(false);
                } else {
                    table[j].writeable(true);
                }
            }
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
    write_cr0(read_cr0() | 1 << 16); // WP bit, makes write protect work in ring0

    auto bss = tos::default_segments::bss();
    auto bss_start = reinterpret_cast<uint64_t*>(bss.base);
    auto bss_end = reinterpret_cast<uint64_t*>(bss.end());

    // Zero out BSS
    std::fill(bss_start, bss_end, 0);

    // Call constructors
    std::for_each(start_ctors, end_ctors, [](auto x) { x(); });

    idt_setup();

    pic::initialize();

    // Initialize the state for SYSCALL/SYSRET instructions.
    tos::x86_64::initialize_syscall_support();

    tos::kern::enable_interrupts();

    tos_main();

    while (true) {
        {
            auto res = tos::global::sched.schedule(tos::int_guard{});
            if (res == tos::exit_reason::restart) {
            }

            if (res == tos::exit_reason::power_down) {
                hlt();
            }
            if (res == tos::exit_reason::idle) {
                hlt();
            }
            if (res == tos::exit_reason::yield) {
            }
        }
    }
}
}