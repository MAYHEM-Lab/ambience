#include <algorithm>
#include <string_view>
#include <tos/compiler.hpp>
#include <tos/i386/assembly.hpp>
#include <tos/i386/mmu.hpp>
#include <tos/multiboot.hpp>
#include <tos/print.hpp>
#include <tos/self_pointing.hpp>
#include <tos/span.hpp>

extern "C" {
extern void (*start_ctors[])(void);
extern void (*end_ctors[])(void);
}
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

class text_vga : public tos::self_pointing<text_vga> {
public:
    static constexpr size_t width = 80;
    static constexpr size_t heigth = 25;

    text_vga() {
        set_color(text_vga_color::white, text_vga_color::black);
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

extern "C" {
alignas(4096) std::array<tos::i386::page_entry, 512> p4_table;
alignas(4096) std::array<tos::i386::page_entry, 512> p3_table;
alignas(4096) std::array<tos::i386::page_entry, 512> p2_table;
}

extern "C" {
void set_up_page_tables() {
    p4_table[0].zero().present(true);
    p4_table[0].writeable(true);
    p4_table[0].page_base(reinterpret_cast<uintptr_t>(&p3_table));

    p3_table[0].zero().present(true);
    p3_table[0].writeable(true);
    p3_table[0].page_base(reinterpret_cast<uintptr_t>(&p2_table));

    // 1G identity mapped
    for (int i = 0; i < 512; ++i) {
        p2_table[i].zero().page_base(i * (1 << 21));
        p2_table[i].present(true);
        p2_table[i].writeable(true);
        p2_table[i].huge_page(true);
    }
}
}

namespace {
NO_INLINE
void enable_paging() {
    using namespace tos::i386;

    write_cr3(reinterpret_cast<uint32_t>(&p4_table));

    auto cr4 = read_cr4();
    cr4 |= 1 << 5;
    write_cr4(cr4);

    auto msr = rdmsr(0xC0000080);
    msr |= 1 << 8;
    wrmsr(0xC0000080, msr);

    auto cr0 = read_cr0();
    cr0 |= 1 << 31;
    write_cr0(cr0);
}
} // namespace
extern uint8_t program[];

extern "C" {

uint64_t gdt_data[3];

struct [[gnu::packed]] {
    uint16_t sz;
    uint32_t ptr;
} gdt;

[[gnu::noinline]] void setup_gdt() {
    memset(&gdt_data, 0, sizeof(gdt_data));

    gdt_data[0] = 0x0000000000000000;
    gdt_data[1] = 0x00209A0000000000;
    gdt_data[2] = 0x0000920000000000;

    gdt.sz = sizeof(gdt_data) - 1;
    gdt.ptr = reinterpret_cast<uint32_t>(&gdt_data);
}

[[noreturn]] [[gnu::used]] void init(uint32_t signature,
                                     const tos::multiboot::info_t* info) {
    tos::x86::text_vga vga;
    if (signature == 0x2BADB002) {
        tos::multiboot::load_info = info;
        vga.write("Loaded from multiboot\n\r");
        tos::println(vga, *info);
    }
    std::for_each(start_ctors, end_ctors, [](auto x) { x(); });
    vga.write("Hello\n\r");
    set_up_page_tables();

    enable_paging();
    vga.write("Switched to long mode\n\r");

    while (true);

    vga.write("Setting up GDT\n\r");
    setup_gdt();
    vga.write("Set up GDT\n\r");

    //    enable_fpu();
    //    vga.write("Enabled FPU\n\r");
    //
    //    enable_sse();
    //    vga.write("Enabled SSE\n\r");
    //
    //    enable_xsave();
    //    vga.write("Enabled XSAVE\n\r");
    //
    //    enable_avx();
    //    vga.write("Enabled AVX\n\r");

    if (reinterpret_cast<uint64_t>(&program) != 4096) {
        // hmm
        vga.write("Bad program position!\n\r");
    }
    vga.write("Init done\n\r");

    vga.write("Loading...\n\r");

    using entry_t = int (*)();
    auto entry = reinterpret_cast<entry_t>(&program[0]);

    asm volatile("lgdt %0" : : "m"(gdt));
    asm volatile("mov %0, %%edi": : "r"(info));
    asm volatile("jmp $0x8,$0x1000");
    auto res = entry();
    TOS_UNREACHABLE();
    char buf[] = "0\n\r";
    buf[0] += res;
    vga.write(buf);
}
}