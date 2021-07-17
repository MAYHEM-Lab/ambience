#include "stivale2.h"
#include <algorithm>
#include <cmath>
#include <string_view>
#include <tos/debug/panic.hpp>
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
#include <tos/x86_64/msr.hpp>
#include <tos/x86_64/pic.hpp>
#include <tos/x86_64/port.hpp>
#include <tos/x86_64/syscall.hpp>
#include <tos/x86_64/tss.hpp>

extern "C" {
void __cxa_atexit(void (*)(void*), void*, void*) {
}
}

extern "C" {
void abort() {
    tos::debug::panic("abort called");
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

[[gnu::section(".nozero")]] tos::stack_storage main_stack{};
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

    // https://en.wikipedia.org/wiki/Control_register#CR4
    auto cr4 = read_cr4();
    cr4 |= (1 << 9) | (1 << 10); // OSFXSR | OSXMMEXCPT
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
NO_INLINE
void set_up_page_tables() {
    memset(&p4_table, 0, sizeof p4_table);
    memset(&p3_table, 0, sizeof p3_table);
    memset(&p2_tables, 0, sizeof p2_tables);
    memset(&p1_tables, 0, sizeof p1_tables);

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
// stivale2 offers a runtime terminal service which can be ditched at any
// time, but it provides an easy way to print out to graphical terminal,
// especially during early boot.
// Read the notes about the requirements for using this feature below this
// code block.
static struct stivale2_header_tag_terminal terminal_hdr_tag = {
    // All tags need to begin with an identifier and a pointer to the next tag.
    .tag =
        {// Identification constant defined in stivale2.h and the specification.
         .identifier = STIVALE2_HEADER_TAG_TERMINAL_ID,
         // If next is 0, it marks the end of the linked list of header tags.
         .next = 0},
    // The terminal header tag possesses a flags field, leave it as 0 for now
    // as it is unused.
    .flags = 0};

// We are now going to define a framebuffer header tag, which is mandatory when
// using the stivale2 terminal.
// This tag tells the bootloader that we want a graphical framebuffer instead
// of a CGA-compatible text mode. Omitting this tag will make the bootloader
// default to text mode, if available.
static struct stivale2_header_tag_framebuffer framebuffer_hdr_tag = {
    // Same as above.
    .tag = {.identifier = STIVALE2_HEADER_TAG_FRAMEBUFFER_ID,
            // Instead of 0, we now point to the previous header tag. The order in
            // which header tags are linked does not matter.
            .next = (uint64_t)&terminal_hdr_tag},
    // We set all the framebuffer specifics to 0 as we want the bootloader
    // to pick the best it can.
    .framebuffer_width = 0,
    .framebuffer_height = 0,
    .framebuffer_bpp = 0};

// The stivale2 specification says we need to define a "header structure".
// This structure needs to reside in the .stivale2hdr ELF section in order
// for the bootloader to find it. We use this __attribute__ directive to
// tell the compiler to put the following structure in said section.
__attribute__((section(".stivale2hdr"),
               used)) static struct stivale2_header stivale_hdr = {
    // The entry_point member is used to specify an alternative entry
    // point that the bootloader should jump to instead of the executable's
    // ELF entry point. We do not care about that so we leave it zeroed.
    .entry_point = 0,
    // Let's tell the bootloader where our stack is.
    // We need to add the sizeof(stack) since in x86(_64) the stack grows
    // downwards.
    .stack = (uintptr_t)&main_stack + sizeof(main_stack),
    // Bit 1, if set, causes the bootloader to return to us pointers in the
    // higher half, which we likely want.
    // Bit 2, if set, tells the bootloader to enable protected memory ranges,
    // that is, to respect the ELF PHDR mandated permissions for the executable's
    // segments.
    .flags = 0,
    // This header structure is the root of the linked list of header tags and
    // points to the first one in the linked list.
    .tags = (uintptr_t)&framebuffer_hdr_tag};

// We will now write a helper function which will allow us to scan for tags
// that we want FROM the bootloader (structure tags).
void* stivale2_get_tag(stivale2_struct* stivale2_struct, uint64_t id) {
    auto current_tag = (stivale2_tag*)stivale2_struct->tags;
    for (;;) {
        // If the tag pointer is NULL (end of linked list), we did not find
        // the tag. Return NULL to signal this.
        if (current_tag == NULL) {
            return NULL;
        }

        // Check whether the identifier matches. If it does, return a pointer
        // to the matching tag.
        if (current_tag->identifier == id) {
            return current_tag;
        }

        // Get a pointer to the next tag in the linked list and repeat.
        current_tag = (stivale2_tag*)current_tag->next;
    }
}

[[noreturn]] [[gnu::section(".text.entry")]] void
_start(stivale2_struct* stivale2_struct) {
    // Let's get the terminal structure tag from the bootloader.
    auto term_str_tag = (stivale2_struct_tag_terminal*)stivale2_get_tag(
        stivale2_struct, STIVALE2_STRUCT_TAG_TERMINAL_ID);

    // Check if the tag was actually found.
    if (term_str_tag == NULL) {
        // It wasn't found, just hang...
        for (;;) {
            asm("hlt");
        }
    }

    // Let's get the address of the terminal write function.
    void* term_write_ptr = (void*)term_str_tag->term_write;

    // Now, let's assign this pointer to a function pointer which
    // matches the prototype described in the stivale2 specification for
    // the stivale2_term_write function.
    auto term_write = (void (*)(const char* string, size_t length))term_write_ptr;

    // We should now be able to call the above function pointer to print out
    // a simple "Hello World" to screen.
    term_write("Hello World", 11);

    set_stack_ptr(reinterpret_cast<char*>(&main_stack) + sizeof main_stack);
    _prestart(nullptr);
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

[[noreturn]] [[gnu::used]] [[gnu::force_align_arg_pointer]] void
_post_start([[maybe_unused]] const tos::multiboot::info_t* info) {
    write_cr0(read_cr0() | 1 << 16); // WP bit, makes write protect work in ring0

    wrmsr(msrs::ia32_efer, rdmsr(msrs::ia32_efer) | 1 << 11); // NX-bit support

    write_cr4(read_cr4() | 1 << 5 | 1 << 7); // PAE

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