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
#include <tos/x86_64/idt.hpp>
#include <tos/x86_64/mmu.hpp>
#include <tos/x86_64/port.hpp>

extern void tos_main();

namespace {
using namespace tos::x86_64;

extern "C" {
void _div_by_zero_handler();
void _debug_handler();
void _nmi_handler();
void _breakpoint_handler();
void _overflow_handler();
void _out_of_bounds_handler();
void _invalid_opcode_handler();
void _device_not_available_handler();
void _double_fault_handler();
void _invalid_tss_handler();
void _coprocessor_seg_overrun_handler();
void _segment_not_present_handler();
void _stack_segment_fault_handler();
void _general_protection_fault_handler();
void _page_fault_handler();
void _x87_fpu_fault_handler();
void _alignment_check_handler();
void _machine_check_handler();
void _simd_fpu_fault_handler();
void _virt_handler();
void _security_exception_handler();
void _irq0_handler();

void irq0_handler(exception_frame* f) {
    port(0x20).outb(0x20);
}
}

interrupt_descriptor_table idt;

struct [[gnu::packed]] {
    uint16_t limits;
    uint64_t base;
} idt_thing;

tos::expected<void, idt_error> idt_setup() {
    idt.div_by_zero = idt_entry::create(_div_by_zero_handler);
    idt.debug = idt_entry::create(_debug_handler);
    idt.nmi = idt_entry::create(_nmi_handler);
    idt.breakpoint = idt_entry::create(_breakpoint_handler);
    idt.overflow = idt_entry::create(_overflow_handler);
    idt.out_of_bounds = idt_entry::create(_out_of_bounds_handler);
    idt.invalid_opcode = idt_entry::create(_invalid_opcode_handler);
    idt.device_not_available = idt_entry::create(_device_not_available_handler);
    idt.double_fault = idt_entry::create(_double_fault_handler);
    idt.invalid_tss = idt_entry::create(_invalid_tss_handler);
    idt.coprocessor_seg_overrun = idt_entry::create(_coprocessor_seg_overrun_handler);
    idt.segment_not_present = idt_entry::create(_segment_not_present_handler);
    idt.stack_segment_fault = idt_entry::create(_stack_segment_fault_handler);
    idt.general_protection_fault = idt_entry::create(_general_protection_fault_handler);
    idt.page_fault = idt_entry::create(_page_fault_handler);
    idt.x87_fpu_fault = idt_entry::create(_x87_fpu_fault_handler);
    idt.alignment_check = idt_entry::create(_alignment_check_handler);
    idt.machine_check = idt_entry::create(_machine_check_handler);
    idt.simd_fpu_fault = idt_entry::create(_simd_fpu_fault_handler);
    idt.virt = idt_entry::create(_virt_handler);
    idt.security_exception = idt_entry::create(_security_exception_handler);

    idt.rest[11] = idt_entry::create(_irq0_handler);

    port(0x20).outb(0x11);
    port(0xA0).outb(0x11);
    port(0x21).outb(0x20);
    port(0xA1).outb(40);
    port(0x21).outb(0x04);
    port(0xA1).outb(0x02);
    port(0x21).outb(0x01);
    port(0xA1).outb(0x01);
    port(0x21).outb(0x0);
    port(0xA1).outb(0x0);

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

struct [[gnu::packed]] gdt_entry {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_mid;
    uint8_t access;
    uint8_t opts_limit_mid;
    uint8_t base_hi;
};

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
[[gnu::section(".nozero")]] alignas(
    4096) std::array<tos::x86_64::table_entry, 512> p2_table;
}

extern "C" {
void set_up_page_tables() {
    memset(&p4_table, 0, sizeof p4_table);
    memset(&p3_table, 0, sizeof p3_table);
    memset(&p2_table, 0, sizeof p2_table);

    p4_table[0].zero().valid(true).writeable(true).page_num(
        reinterpret_cast<uintptr_t>(&p3_table));

    p3_table[0].zero().valid(true).writeable(true).page_num(
        reinterpret_cast<uintptr_t>(&p2_table));

    // 1G identity mapped
    for (int i = 0; i < 512; ++i) {
        p2_table[i]
            .zero()
            .page_num(i * (1 << 21))
            .valid(true)
            .writeable(true)
            .huge_page(true);
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