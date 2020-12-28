#include <algorithm>
#include <string_view>
#include <tos/compiler.hpp>
#include <tos/i386/assembly.hpp>
#include <tos/i386/mmu.hpp>
#include <tos/multiboot.hpp>
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

class text_vga {
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
    p4_table[0].present(true);
    p4_table[0].writeable(true);
    p4_table[0].page_base(reinterpret_cast<uintptr_t>(&p3_table));

    p3_table[0].present(true);
    p3_table[0].writeable(true);
    p3_table[0].page_base(reinterpret_cast<uintptr_t>(&p2_table));

    // 1G identity mapped
    for (int i = 0; i < 512; ++i) {
        p2_table[i].page_base(i * (1 << 21));
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

NO_INLINE
void enable_avx() {
    using namespace tos::i386;

    auto cr4 = read_cr4();
    cr4 |= (1 << 14);
    write_cr4(cr4);
}

NO_INLINE
void enable_xsave() {
    using namespace tos::i386;

    auto cr4 = read_cr4();
    cr4 |= (1 << 18);
    write_cr4(cr4);

    auto r0 = xgetbv(0);
//    r0 |= 0x3;
//    xsetbv(0, r0);
}

NO_INLINE
void enable_fpu() {
    using namespace tos::i386;

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
    using namespace tos::i386;

    auto cr4 = read_cr4();
    cr4 |= (1 << 9) | (1 << 10);
    write_cr4(cr4);
}
} // namespace

[[gnu::section(".to_load")]] uint8_t program[] = {
    0xb8, 0x00, 0x80, 0x0b, 0x00, 0x31, 0xc9, 0x31, 0xd2, 0x66, 0xc7, 0x04, 0x50, 0x20,
    0x0f, 0x48, 0xff, 0xc2, 0x48, 0x83, 0xfa, 0x50, 0x75, 0xf1, 0x48, 0xff, 0xc1, 0x48,
    0x05, 0xa0, 0x00, 0x00, 0x00, 0x48, 0x83, 0xf9, 0x19, 0x75, 0xe0, 0xb8, 0x03, 0x00,
    0x00, 0x00, 0xc3, 0x53, 0x48, 0x89, 0xfb, 0xe8, 0x25, 0x00, 0x00, 0x00, 0x48, 0x8b,
    0x08, 0x48, 0x89, 0xc7, 0xbe, 0x01, 0x00, 0x00, 0x00, 0xff, 0x51, 0x10, 0x48, 0x85,
    0xc0, 0x74, 0x10, 0x48, 0x8b, 0x38, 0x48, 0x8b, 0x07, 0x48, 0x8b, 0x40, 0x08, 0x48,
    0x89, 0xde, 0x5b, 0xff, 0xe0, 0x5b, 0xc3, 0x48, 0x8b, 0x3d, 0xee, 0x0f, 0x00, 0x00,
    0x48, 0x85, 0xff, 0x0f, 0x85, 0x05, 0x00, 0x00, 0x00, 0xe9, 0x25, 0x00, 0x00, 0x00,
    0x48, 0x8b, 0x47, 0x30, 0xc3, 0x50, 0xc6, 0x05, 0xe3, 0x0f, 0x00, 0x00, 0x00, 0xe8,
    0x13, 0x00, 0x00, 0x00, 0x48, 0x89, 0x05, 0xdf, 0x0f, 0x00, 0x00, 0x0f, 0x57, 0xc0,
    0x0f, 0x11, 0x05, 0xdd, 0x0f, 0x00, 0x00, 0x58, 0xc3, 0x53, 0x48, 0x83, 0xec, 0x70,
    0x80, 0x3d, 0x17, 0x10, 0x00, 0x00, 0x00, 0x74, 0x14, 0x80, 0x3d, 0x3e, 0x10, 0x00,
    0x01, 0x00, 0x74, 0x68, 0xb8, 0xc0, 0x20, 0x00, 0x01, 0x48, 0x83, 0xc4, 0x70, 0x5b,
    0xc3, 0x48, 0x8d, 0x5c, 0x24, 0x30, 0xbe, 0xc0, 0x20, 0x00, 0x00, 0xba, 0x00, 0x00,
    0x00, 0x01, 0x48, 0x89, 0xdf, 0xe8, 0x04, 0x01, 0x00, 0x00, 0x0f, 0x10, 0x03, 0x0f,
    0x10, 0x4b, 0x10, 0x0f, 0x10, 0x53, 0x20, 0x0f, 0x29, 0x14, 0x24, 0x0f, 0x29, 0x44,
    0x24, 0x20, 0x0f, 0x28, 0x14, 0x24, 0x0f, 0x29, 0x54, 0x24, 0x10, 0x48, 0xc7, 0x05,
    0x8a, 0x0f, 0x00, 0x00, 0x38, 0x14, 0x00, 0x00, 0x0f, 0x11, 0x05, 0x8b, 0x0f, 0x00,
    0x00, 0x0f, 0x11, 0x0d, 0x94, 0x0f, 0x00, 0x00, 0x0f, 0x11, 0x15, 0x9d, 0x0f, 0x00,
    0x00, 0xc6, 0x05, 0xa6, 0x0f, 0x00, 0x00, 0x01, 0xeb, 0x8f, 0xe8, 0x67, 0x00, 0x00,
    0x00, 0x48, 0xc7, 0x05, 0x9c, 0x0f, 0x00, 0x01, 0x70, 0x14, 0x00, 0x00, 0x48, 0xc7,
    0x05, 0xa9, 0x0f, 0x00, 0x01, 0x80, 0x20, 0x00, 0x00, 0x48, 0x89, 0x05, 0xaa, 0x0f,
    0x00, 0x01, 0xc6, 0x05, 0xab, 0x0f, 0x00, 0x01, 0x01, 0xe9, 0x6a, 0xff, 0xff, 0xff,
    0xc3, 0xcc, 0x48, 0x83, 0xc7, 0x08, 0xe9, 0xa3, 0x00, 0x00, 0x00, 0xcc, 0x48, 0x83,
    0xc7, 0x08, 0xe9, 0x33, 0x02, 0x00, 0x00, 0xcc, 0x48, 0x8b, 0x47, 0x28, 0xb2, 0x01,
    0xc3, 0xcc, 0xe9, 0xc8, 0xfe, 0xff, 0xff, 0xcc, 0x48, 0x89, 0xf8, 0x83, 0xfe, 0x2a,
    0x74, 0x0a, 0x83, 0xfe, 0x01, 0x75, 0x0a, 0x48, 0x83, 0xc0, 0x18, 0xc3, 0x48, 0x83,
    0xc0, 0x20, 0xc3, 0x31, 0xc0, 0xc3, 0x80, 0x3d, 0x71, 0x0f, 0x00, 0x01, 0x00, 0x74,
    0x0f, 0x80, 0x3d, 0x88, 0x0f, 0x00, 0x01, 0x00, 0x74, 0x1a, 0xb8, 0x00, 0x21, 0x00,
    0x01, 0xc3, 0x48, 0xc7, 0x05, 0x4d, 0x0f, 0x00, 0x01, 0xa8, 0x14, 0x00, 0x00, 0xc6,
    0x05, 0x4e, 0x0f, 0x00, 0x01, 0x01, 0xeb, 0xdd, 0xb0, 0x01, 0x88, 0x05, 0x4c, 0x0f,
    0x00, 0x01, 0x48, 0xc7, 0x05, 0x49, 0x0f, 0x00, 0x01, 0xf0, 0x20, 0x00, 0x01, 0xc6,
    0x05, 0x4a, 0x0f, 0x00, 0x01, 0x00, 0x88, 0x05, 0x4c, 0x0f, 0x00, 0x01, 0xeb, 0xc4,
    0x31, 0xc0, 0xc3, 0xcc, 0x48, 0x89, 0x37, 0x48, 0x89, 0x57, 0x08, 0x0f, 0x57, 0xc0,
    0x0f, 0x11, 0x47, 0x20, 0x48, 0x89, 0x56, 0x10, 0x48, 0x89, 0x77, 0x10, 0x48, 0x89,
    0x77, 0x18, 0x0f, 0x11, 0x06, 0xc3, 0x41, 0x57, 0x41, 0x56, 0x41, 0x54, 0x53, 0x50,
    0x89, 0xf0, 0x83, 0xe0, 0x0f, 0xb9, 0x10, 0x00, 0x00, 0x00, 0x48, 0x29, 0xc1, 0x48,
    0x85, 0xc0, 0x48, 0x0f, 0x44, 0xc8, 0x48, 0x8d, 0x04, 0x0e, 0x48, 0x83, 0xc0, 0x10,
    0x48, 0x83, 0xf8, 0x18, 0x41, 0xbc, 0x18, 0x00, 0x00, 0x00, 0x4c, 0x0f, 0x47, 0xe0,
    0x48, 0x8b, 0x5f, 0x10, 0x48, 0x85, 0xdb, 0x74, 0x1b, 0x49, 0x89, 0xff, 0x48, 0x83,
    0xc7, 0x10, 0x45, 0x31, 0xf6, 0x4c, 0x39, 0x63, 0x10, 0x73, 0x10, 0x48, 0x8b, 0x5b,
    0x08, 0x48, 0x85, 0xdb, 0x75, 0xf1, 0xeb, 0x55, 0x45, 0x31, 0xf6, 0xeb, 0x50, 0x48,
    0x89, 0xde, 0xe8, 0x57, 0x00, 0x00, 0x00, 0x4c, 0x8d, 0x73, 0x10, 0x48, 0x8b, 0x43,
    0x10, 0x49, 0x8d, 0x4c, 0x24, 0x18, 0x48, 0x39, 0xc8, 0x73, 0x05, 0x49, 0x89, 0xc4,
    0xeb, 0x14, 0x4a, 0x8d, 0x34, 0x23, 0x4c, 0x29, 0xe0, 0x4a, 0x89, 0x44, 0x23, 0x10,
    0x4c, 0x89, 0xff, 0xe8, 0x82, 0x00, 0x00, 0x00, 0x49, 0x8b, 0x47, 0x20, 0x4c, 0x01,
    0xe0, 0x49, 0x89, 0x47, 0x20, 0x49, 0x8b, 0x4f, 0x28, 0x48, 0x39, 0xc8, 0x48, 0x0f,
    0x42, 0xc1, 0x49, 0x89, 0x47, 0x28, 0x4c, 0x89, 0x23, 0x4c, 0x89, 0xf0, 0x48, 0x83,
    0xc4, 0x08, 0x5b, 0x41, 0x5c, 0x41, 0x5e, 0x41, 0x5f, 0xc3, 0x48, 0x8b, 0x0f, 0x48,
    0x8b, 0x47, 0x08, 0x48, 0x39, 0xc1, 0x74, 0x1e, 0x48, 0x39, 0xf1, 0x74, 0x21, 0x48,
    0x39, 0xf0, 0x74, 0x2c, 0x48, 0x8b, 0x06, 0x48, 0x8b, 0x4e, 0x08, 0x48, 0x89, 0x48,
    0x08, 0x48, 0x8b, 0x4e, 0x08, 0x48, 0x89, 0x01, 0xeb, 0x27, 0x0f, 0x57, 0xc0, 0x0f,
    0x11, 0x07, 0xeb, 0x1f, 0x48, 0x8b, 0x46, 0x08, 0x48, 0x89, 0x07, 0x48, 0xc7, 0x00,
    0x00, 0x00, 0x00, 0x00, 0xeb, 0x0f, 0x48, 0x8b, 0x06, 0x48, 0x89, 0x47, 0x08, 0x48,
    0xc7, 0x40, 0x08, 0x00, 0x00, 0x00, 0x00, 0x48, 0x8b, 0x46, 0x08, 0xc3, 0x41, 0x56,
    0x53, 0x50, 0x48, 0x89, 0xf2, 0x49, 0x89, 0xfe, 0x48, 0x8b, 0x77, 0x10, 0x49, 0x83,
    0xc6, 0x10, 0x48, 0x85, 0xf6, 0x74, 0x0b, 0x48, 0x39, 0xd6, 0x77, 0x06, 0x48, 0x8b,
    0x76, 0x08, 0xeb, 0xf0, 0x4c, 0x89, 0xf7, 0xe8, 0x86, 0x00, 0x00, 0x00, 0x48, 0x8b,
    0x70, 0x08, 0x48, 0x85, 0xf6, 0x74, 0x35, 0x48, 0x89, 0xc3, 0x48, 0x8b, 0x40, 0x10,
    0x48, 0x8d, 0x0c, 0x03, 0x48, 0x39, 0xf1, 0x75, 0x10, 0x48, 0x03, 0x46, 0x10, 0x48,
    0x89, 0x43, 0x10, 0x4c, 0x89, 0xf7, 0xe8, 0x57, 0xff, 0xff, 0xff, 0x49, 0x3b, 0x1e,
    0x74, 0x10, 0x48, 0x8b, 0x03, 0x48, 0x8b, 0x48, 0x10, 0x48, 0x8d, 0x14, 0x08, 0x48,
    0x39, 0xda, 0x74, 0x08, 0x48, 0x83, 0xc4, 0x08, 0x5b, 0x41, 0x5e, 0xc3, 0x48, 0x03,
    0x4b, 0x10, 0x48, 0x89, 0x48, 0x10, 0x4c, 0x89, 0xf7, 0x48, 0x89, 0xde, 0x48, 0x83,
    0xc4, 0x08, 0x5b, 0x41, 0x5e, 0xe9, 0x20, 0xff, 0xff, 0xff, 0x41, 0x56, 0x53, 0x50,
    0x49, 0x89, 0xfe, 0x48, 0x8d, 0x46, 0xf0, 0x48, 0x8b, 0x5e, 0xf0, 0x48, 0x89, 0x1e,
    0x48, 0x89, 0xc6, 0xe8, 0x5c, 0xff, 0xff, 0xff, 0x49, 0x29, 0x5e, 0x20, 0x48, 0x83,
    0xc4, 0x08, 0x5b, 0x41, 0x5e, 0xc3, 0x48, 0x89, 0xd0, 0x48, 0x8b, 0x0f, 0x48, 0x39,
    0xf1, 0x74, 0x1f, 0x48, 0x85, 0xf6, 0x74, 0x28, 0x48, 0x8b, 0x0e, 0x48, 0x89, 0x08,
    0x48, 0x89, 0x06, 0x48, 0x89, 0x70, 0x08, 0x48, 0x8b, 0x08, 0x48, 0x85, 0xc9, 0x74,
    0x04, 0x48, 0x89, 0x41, 0x08, 0xc3, 0x48, 0x85, 0xf6, 0x74, 0x1b, 0x48, 0x89, 0x70,
    0x08, 0x48, 0x89, 0x06, 0xeb, 0x1e, 0x48, 0x85, 0xc9, 0x74, 0x24, 0x48, 0x8b, 0x4f,
    0x08, 0x48, 0x89, 0x08, 0x48, 0x89, 0x41, 0x08, 0xeb, 0x21, 0x48, 0x89, 0x47, 0x08,
    0x48, 0xc7, 0x40, 0x08, 0x00, 0x00, 0x00, 0x00, 0x48, 0x89, 0x07, 0x48, 0xc7, 0x00,
    0x00, 0x00, 0x00, 0x00, 0xc3, 0x48, 0x89, 0x07, 0x48, 0xc7, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x48, 0x89, 0x47, 0x08, 0x48, 0xc7, 0x40, 0x08, 0x00, 0x00, 0x00, 0x00, 0xc3,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x44, 0x11, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x4e, 0x11, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x58, 0x11, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x42, 0x11,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60, 0x11, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x42, 0x11, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60, 0x11, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x66, 0x11, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x42, 0x11,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x42, 0x11, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0xce, 0x11, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x42, 0x11, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x42, 0x11, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x42, 0x11,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x42, 0x11, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x42, 0x11, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x42, 0x11, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x42, 0x11, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x42, 0x11, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x42, 0x11, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60, 0x11,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0x75, 0x10, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00};

extern "C" {
[[gnu::used]] void init(uint32_t signature, const tos::multiboot::info_t* info) {
    std::for_each(start_ctors, end_ctors, [](auto x) { x(); });
    tos::x86::text_vga vga;
    vga.write("Hello\n\r");
    set_up_page_tables();

    enable_paging();
    vga.write("Switched to long mode\n\r");
    if (signature == 0x2BADB002) {
        tos::multiboot::load_info = info;
        vga.write("Loaded from multiboot\n\r");
    }

    enable_fpu();
    vga.write("Enabled FPU\n\r");

    enable_sse();
    vga.write("Enabled SSE\n\r");

    enable_xsave();
    vga.write("Enabled XSAVE\n\r");
    //
    //    enable_avx();
    //    vga.write("Enabled AVX\n\r");


    if (reinterpret_cast<uint64_t>(&program) != 4096) {
        // hmm
        vga.write("Bad program position!\n\r");
    }
    vga.write("Init done\n\r");

    while (true);
    vga.write("Loading...\n\r");

    using entry_t = int (*)();
    auto entry = reinterpret_cast<entry_t>(&program[0]);
    auto res = entry();
    char buf[] = "0\n\r";
    buf[0] += res;
    vga.write(buf);
}
}