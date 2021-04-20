#include <algorithm>
#include <string_view>
#include <tos/compiler.hpp>
#include <tos/elf.hpp>
#include <tos/i386/assembly.hpp>
#include <tos/i386/mmu.hpp>
#include <tos/multiboot.hpp>
#include <tos/print.hpp>
#include <tos/self_pointing.hpp>
#include <tos/span.hpp>

void tos_force_get_failed(void*) {
    while(true);
}

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
    memset(&p4_table, 0, sizeof p4_table);
    memset(&p3_table, 0, sizeof p3_table);
    memset(&p2_table, 0, sizeof p2_table);

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
void enable_paging(tos::x86::text_vga& vga) {
    using namespace tos::i386;

    auto cr4 = read_cr4();
    cr4 |= 1 << 5 | 1 << 7; // PAE | PGE
    write_cr4(cr4);
    vga.write("Wrote CR4\n\r");

    write_cr3(reinterpret_cast<uint32_t>(&p4_table));
    vga.write("Wrote CR3\n\r");

    auto msr = rdmsr(0xC0000080); // IA32_EFER
    vga.write("Read MSR\n\r");
    msr |= 1 << 8; // Long mode active
    wrmsr(0xC0000080, msr);
    vga.write("Wrote MSR\n\r");

    auto cr0 = read_cr0();
    cr0 |= 1 << 31; // Paging
    write_cr0(cr0);
    vga.write("Wrote CR0\n\r");
}
} // namespace
extern tos::span<const uint8_t> program_span;

extern "C" {

struct [[gnu::packed]] gdt_entry {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_mid;
    uint8_t access;
    uint8_t opts_limit_mid;
    uint8_t base_hi;
};

gdt_entry gdt_entry_data[3];

struct [[gnu::packed]] {
    uint16_t sz;
    uint32_t ptr;
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
    gdt.ptr = reinterpret_cast<uint32_t>(&gdt_entry_data);
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
    vga.write("Hello world\n\r");
    set_up_page_tables();
    vga.write("Enabling paging\n\r");

    enable_paging(vga);
    vga.write("Switched to long mode\n\r");

    vga.write("Setting up GDT\n\r");
    setup_gdt();
    vga.write("Set up GDT\n\r");

    vga.write("Init done\n\r");

    vga.write("Loading ELF...\n\r");

    vga.write(tos::itoa(program_span.size()).data());
    vga.write("\n\r");

    auto res = tos::elf::elf64::from_buffer(program_span);
    if (!res) {
        vga.write("Could not parse payload\n\r");
        vga.write("Error code: ");
        vga.write(tos::itoa(int(force_error(res))).data());
        vga.write("\n\r");
        while (true)
            ;
    }

    auto& elf = force_get(res);
    tos::println(vga, "Entry point:", int(elf.header().entry));
    tos::println(vga, (int)elf.header().pheader_offset, (int)elf.header().pheader_size);

    for (auto pheader : elf.program_headers()) {
        tos::println(vga,
                     "Load",
                     (void*)(uint32_t)pheader.file_size,
                     "bytes from",
                     (void*)(uint32_t)pheader.file_offset,
                     "to",
                     (void*)(uint32_t)pheader.virt_address);

        auto seg = elf.segment(pheader);
        tos::println(vga, seg.slice(0, 32));
        memcpy(reinterpret_cast<void*>(pheader.virt_address), seg.data(), seg.size());
    }

    tos::println(vga, "Jumping...");

    asm volatile("lgdt %0" : : "m"(gdt));
    asm volatile("mov %0, %%edi" : : "r"(info));
    asm("push $0x8\n"
        "push %%ecx\n"
        "lret"
        :
        : "c"(uint32_t(elf.header().entry)));
    TOS_UNREACHABLE();
}
}
