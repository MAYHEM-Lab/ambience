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

extern void tos_main();

namespace tos::x86_64 {
struct port {
    constexpr port(uint16_t port_addr)
        : m_port{port_addr} {
    }

    void outb(uint8_t b) {
        asm volatile("outb %0, %1" : : "a"(b), "Nd"(m_port));
    }

    inline uint8_t inb() {
        uint8_t ret;
        asm volatile("inb %1, %0" : "=a"(ret) : "Nd"(m_port));
        return ret;
    }

    void outw(uint16_t w) {
        asm volatile("outw %0, %1" : : "a"(w), "Nd"(m_port));
    }

    void outl(uint32_t l) {
        asm volatile("outl %0, %1" : : "a"(l), "Nd"(m_port));
    }

private:
    uint16_t m_port;
};
} // namespace tos::x86_64
namespace tos::x86 {
enum class text_vga_color : uint8_t
{
    black = 0,
    blue = 1,
    green = 2,
    cyan = 3,
    red = 4,
    magenta = 5,
    brown = 6,
    light_grey = 7,
    dark_grey = 8,
    light_blue = 9,
    light_green = 10,
    light_cyan = 11,
    light_red = 12,
    light_magenta = 13,
    light_brown = 14,
    white = 15,
};

class text_vga {
public:
    static constexpr size_t width = 80;
    static constexpr size_t heigth = 25;

    text_vga() {
        set_color(text_vga_color::white, text_vga_color::black);
    }

    void clear() {
        for (size_t y = 0; y < heigth; y++) {
            for (size_t x = 0; x < width; x++) {
                auto index = y * width + x;
                terminal_buffer[index] = vga_entry(' ', terminal_color);
            }
        }
    }

    void set_color(text_vga_color fg, text_vga_color bg) {
        terminal_color = vga_entry_color(fg, bg);
    }

    void write(std::string_view str) {
        for (auto c : str) {
            put_char(c);
        }
    }

    int write(span<const uint8_t> data) {
        for (auto c : data) {
            put_char(c);
        }
        return data.size();
    }

private:
    static constexpr uint8_t vga_entry_color(text_vga_color fg, text_vga_color bg) {
        return uint8_t(fg) | uint8_t(bg) << 4;
    }

    static constexpr uint16_t vga_entry(uint8_t uc, uint8_t color) {
        return (uint16_t)uc | (uint16_t)color << 8;
    }

    void put_char(char c) {
        if (c == 0) {
            return;
        }

        if (c == '\n') {
            if (++terminal_row == heigth) {
                terminal_row = 0;
            }
            return;
        }

        if (c == '\r') {
            terminal_column = 0;
            return;
        }

        write_at(c, terminal_column, terminal_row);

        if (++terminal_column == width) {
            terminal_column = 0;
            if (++terminal_row == heigth) {
                terminal_row = 0;
            }
        }
    }

    void write_at(char c, int x, int y) {
        const size_t index = y * width + x;
        terminal_buffer[index] = vga_entry(c, terminal_color);
    }

    uint16_t terminal_row = 0;
    uint16_t terminal_column = 0;
    uint8_t terminal_color;
    uint16_t* terminal_buffer = reinterpret_cast<uint16_t*>(0xB8000);
};
} // namespace tos::x86

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

namespace tos::x86_64 {
struct uart_16550 : tos::self_pointing<uart_16550> {
public:
    static expected<uart_16550, nullptr_t> open(uint16_t base_port = 0x3F8) {
        auto res = uart_16550();
        res.m_base_port = base_port;
        res.int_en().outb(0);
        res.line_ctrl().outb(0x80); // Enable DLAB, maps 0 and 1 registers to divisor

        res.div_lo().outb(3); // 38400 baud
        res.div_hi().outb(0);

        res.line_ctrl().outb(0x03);  // 8 bits, no parity, one stop bit
        res.fifo_ctrl().outb(0xc7);  // Enable FIFO, clear them, with 14-byte threshold
        res.modem_ctrl().outb(0x0b); // IRQs enabled, RTS/DSR set

        res.modem_ctrl().outb(0x1e);

        res.data().outb(0xf6);

        if (res.data().inb() != 0xf6) {
            return unexpected(nullptr);
        }

        res.modem_ctrl().outb(0x0f);

        return res;
    }

    void write(uint8_t byte) {
        data().outb(byte);
    }

    int write(span<const uint8_t> data) {
        for (auto c : data) {
            this->data().outb(c);
        }
        return data.size();
    }

private:
    port data() const {
        return {m_base_port};
    }
    port int_en() const {
        return port(m_base_port + 1);
    }

    port div_lo() const {
        return port(m_base_port);
    }

    port div_hi() const {
        return port(m_base_port + 1);
    }

    port fifo_ctrl() const {
        return port(m_base_port + 2);
    }
    port line_ctrl() const {
        return port(m_base_port + 3);
    }
    port modem_ctrl() const {
        return port(m_base_port + 4);
    }
    port line_sts() const {
        return port(m_base_port + 5);
    }

    uint16_t m_base_port;
};
} // namespace tos::x86_64

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

    auto serial_res = uart_16550::open();
    if (!serial_res) {
        while (true)
            ;
    }

    auto& serial = force_get(serial_res);
    tos::println(serial, "foo");
    //    serial->write(raw_cast(tos::span<const char>("fo"o")));
    //    serial->write(raw_cast(tos::span<const char>("\n\r")));

    tos::x86::text_vga vga;
    vga.clear();
    vga.write("Yolo from x64\n\r");

    tos::println(serial, (void*)info);
    tos::println(serial, (void*)info->flags);
    tos::println(serial, info->drives_length);
    tos::println(serial, "Bootloader:", (const char*)info->boot_loader_name);
    tos::println(serial, "Command line:", (const char*)info->cmdline);

    tos::println(serial, "Enabling FPU");
    enable_fpu();
    tos::println(serial, "FPU Enabled");

    tos::println(serial, "Enabling SSE");
    enable_sse();
    tos::println(serial, "SSE Enabled");

    auto bss = tos::default_segments::bss();
    auto bss_start = reinterpret_cast<char*>(bss.base);
    auto bss_end = reinterpret_cast<char*>(bss.end());

    auto stack_region =
        tos::memory_range{reinterpret_cast<uintptr_t>(&main_stack), sizeof main_stack};

    tos::println(serial, "Zeroing BSS", (void*)bss_start, (void*)bss_end);
    for (auto it = bss_start; it != bss_end; ++it) {
        if (tos::contains(stack_region, reinterpret_cast<uintptr_t>(it))) {
            continue;
        }
        *it = 0;
    }

    tos::println(serial, "BSS Zeroed");

    tos::println(serial, "Setting up IDT");
    idt_setup();
    tos::println(serial, "IDT Set up");

    vga.write("Booted\n\r");
    tos::println(serial, (void*)start_ctors, (void*)end_ctors);
    std::for_each(start_ctors, end_ctors, [](auto x) { x(); });
    tos::println(serial, "Constructors ran", (int)tos::global::disable_depth);

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