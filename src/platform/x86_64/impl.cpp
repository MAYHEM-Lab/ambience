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
#include <tos/x86_64/port.hpp>

extern void tos_main();

namespace {
using namespace tos::x86_64;

enum pic_cmd_codes : uint8_t
{
    icw4 = 0x1,
    init = 0x10,
    end_of_interrupt = 0x20
};

extern "C" {
void irq0_handler(exception_frame* f) {
    port(0x20).outb(end_of_interrupt);
}
void irq1_handler(exception_frame* f) {
    port(0x20).outb(end_of_interrupt);
}
}

auto idt = default_idt();

struct [[gnu::packed]] {
    uint16_t limits;
    uint64_t base;
} idt_thing;

inline void io_wait() {
    asm volatile("outb %%al, $0x80" : : "a"(0));
}

class pic {
public:
    void doit() {
        // init with ICW4 needed (0x10 = init, 0x01 = read icw4)
        master_cmd().outb(0x11);
        io_wait();
        slave_cmd().outb(0x11);
        io_wait();

        // ICW2
        master_data().outb(0x20); // IRQ offset at 32
        io_wait();
        slave_data().outb(0x28); // IRQ offset at 40
        io_wait();

        // ICW3
        master_data().outb(0x04); // there is a slave at IRQ2 (0100)
        io_wait();
        slave_data().outb(0x02); // slave id is 2
        io_wait();

        // ICW4 = we're in 8086 mode
        master_data().outb(0x01);
        io_wait();
        slave_data().outb(0x01);
        io_wait();

        // Masks, all disabled
        master_data().outb(0xff);
        slave_data().outb(0xff);
    }

    void enable_irq(int irq) {
        if (irq >= 8) {
            slave_data().outb(slave_data().inb() & ~(1 << (irq - 8)));
            return;
        }
        master_data().outb(master_data().inb() & ~(1 << (irq)));
    }

    void disable_irq(int irq) {
        if (irq >= 8) {
            slave_data().outb(slave_data().inb() | 1 << (irq - 8));
            return;
        }
        master_data().outb(master_data().inb() | 1 << (irq));
    }

private:
    port master_cmd() const {
        return {0x20};
    }

    port master_data() const {
        return {0x21};
    }

    port slave_cmd() const {
        return {0xA0};
    }

    port slave_data() const {
        return {0xA1};
    }
};

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
[[gnu::section(".nozero")]] alignas(
    4096) std::array<tos::x86_64::table_entry, 512> p2_table;
[[gnu::section(".nozero")]] translation_table p1_tables[2];
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

    for (int i = 0; i < 2; ++i) {
        p2_table[i]
            .zero()
            .page_num(reinterpret_cast<uintptr_t>(&p1_tables[i]))
            .valid(true)
            .writeable(true);
    }

    for (int i = 0; i < 2; ++i) {
        auto& table = p1_tables[i];
        for (int j = 0; j < 512; ++j) {
            if (i == 0 && j == 0) {
                // Catch nullptr accesses.
                table[j].zero();
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

    auto p = pic();
    p.doit();

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