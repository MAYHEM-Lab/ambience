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
#include <tos/x86_64/port.hpp>

extern void tos_main();

namespace {
using namespace tos::x86_64;

[[gnu::interrupt]] void breakpoint_handler(interrupt_stack_frame_t* stack_frame) {
    while (true)
        ;
}

[[gnu::interrupt]] void double_fault_handler(interrupt_stack_frame_t* stack_frame,
                                             unsigned long int err) {
    while (true)
        ;
}

[[gnu::interrupt]] void irq0_handler(interrupt_stack_frame_t* frame) {
    port(0x20).outb(0x20);
}

interrupt_descriptor_table idt;

struct [[gnu::packed]] {
    uint16_t limits;
    uint64_t base;
} idt_thing;

struct [[gnu::packed]] {
    uint16_t sz;
    uint64_t ptr;
} gdt_thing;

tos::expected<void, idt_error> idt_setup() {
    idt.debug = EXPECTED_TRY(idt_entry<interrupt_handler_t>::create(breakpoint_handler));
    idt.breakpoint =
        EXPECTED_TRY(idt_entry<interrupt_handler_t>::create(breakpoint_handler));
    idt.double_fault =
        EXPECTED_TRY(idt_entry<exception_handler_t>::create(double_fault_handler));
    idt.invalid_opcode =
        EXPECTED_TRY(idt_entry<interrupt_handler_t>::create(breakpoint_handler));
    idt.page_fault =
        EXPECTED_TRY(idt_entry<interrupt_handler_t>::create(breakpoint_handler));
    idt.general_protection_fault =
        EXPECTED_TRY(idt_entry<exception_handler_t>::create(double_fault_handler));
    idt.rest[12] = EXPECTED_TRY(idt_entry<interrupt_handler_t>::create(irq0_handler));
    idt.rest[13] = EXPECTED_TRY(idt_entry<interrupt_handler_t>::create(irq0_handler));
    
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

tos::stack_storage<4096 * 8> main_stack{};
} // namespace

namespace {
NO_INLINE
void enable_avx() {
    using namespace tos::x86_64;

    auto cr4 = read_cr4();
    cr4 |= (1 << 14);
    write_cr4(cr4);
}

NO_INLINE
void enable_xsave() {
    using namespace tos::x86_64;

    auto cr4 = read_cr4();
    cr4 |= (1 << 18);
    write_cr4(cr4);

    auto r0 = xgetbv(0);
    //    r0 |= 0x3;
    //    xsetbv(0, r0);
}

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
} // namespace

extern "C" {
extern void (*start_ctors[])(void);
extern void (*end_ctors[])(void);

[[noreturn]] void _post_start(const tos::multiboot::info_t* info);

[[noreturn]] [[gnu::section(".text.entry")]] void
_start(const tos::multiboot::info_t* info) {
    set_stack_ptr(reinterpret_cast<char*>(&main_stack) + sizeof main_stack);
    _post_start(info);
}

[[noreturn]] void _post_start(const tos::multiboot::info_t* info) {
    asm volatile("mov %0, %%ds" : : "r"(0x10));
    asm volatile("mov %0, %%es" : : "r"(0x10));
    asm volatile("mov %0, %%fs" : : "r"(0x10));
    asm volatile("mov %0, %%gs" : : "r"(0x10));
    asm volatile("mov %0, %%ss" : : "r"(0x10));

    enable_fpu();

    enable_sse();

    auto bss = tos::default_segments::bss();
    auto bss_start = reinterpret_cast<char*>(bss.base);
    auto bss_end = reinterpret_cast<char*>(bss.end());

    auto stack_region =
        tos::memory_range{reinterpret_cast<uintptr_t>(&main_stack), sizeof main_stack};

    for (auto it = bss_start; it != bss_end; ++it) {
        if (tos::contains(stack_region, reinterpret_cast<uintptr_t>(it))) {
            continue;
        }
        *it = 0;
    }
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