#pragma once

#include <cstdint>
#include <tos/print.hpp>

namespace tos::multiboot {
enum class header_flags : uint32_t
{
    none = 0,
    page_align = 1,
    memory = 2,
    video = 4,
    all = 7
};

struct header {
    const uint32_t magic = 0x1badb002;
    header_flags flags = header_flags::none;
    uint32_t checksum = -(magic + uint32_t(flags));
};

struct aout_symbol_table_t {
    uint32_t tabsize;
    uint32_t strsize;
    uint32_t addr;
    uint32_t reserved;
};

struct elf_section_header_table_t {
    uint32_t num;
    uint32_t size;
    uint32_t addr;
    uint32_t shndx;
};

enum class framebuffer_type : uint8_t
{
    indexed,
    rgb,
    ega_text
};

enum class memory
{
    available = 1,
    reserved = 2,
    acpi_reclaimable = 3,
    nvs = 4,
    badram = 5
};

struct memory_map_t {
    uint32_t size;
    uint64_t addr;
    uint64_t len;
    uint32_t type;
} __attribute__((packed));

enum class info_flags : uint32_t
{
    has_bootloader_name = 1 << 9,
    has_apm = 1 << 10,
    has_vbe = 1 << 11,
    has_framebuffer = 1 << 12
};

inline info_flags operator&(info_flags left, info_flags right) {
    return info_flags(uint32_t(left) & uint32_t(right));
}

struct apm_table_t {
    uint16_t version;
    uint16_t cseg;
    uint32_t offset;
    uint16_t cseg_16;
    uint16_t dseg;
    uint16_t flags;
    uint16_t cseg_len;
    uint16_t cseg_16_len;
    uint16_t dseg_len;
} __attribute__((packed));

static_assert(sizeof(apm_table_t) == 20);

struct info_t {
    // Multiboot info version number.
    info_flags flags;

    // Available memory from BIOS.
    uint32_t mem_lower;
    uint32_t mem_upper;

    // "root" partition.
    uint32_t boot_device;

    // Kernel command line.
    uint32_t cmdline;

    // Boot-Module list.
    uint32_t mods_count;
    uint32_t mods_addr;

    union {
        aout_symbol_table_t aout_sym;
        elf_section_header_table_t elf_sec;
    } u;

    // Memory Mapping buffer.
    uint32_t mmap_length;
    uint32_t mmap_addr;

    // Drive Info buffer.
    uint32_t drives_length;
    uint32_t drives_addr;

    // ROM configuration table.
    uint32_t config_table;

    // Boot Loader Name.
    uint32_t boot_loader_name;

    // APM table.
    const apm_table_t* apm_table;

    // Video.
    uint32_t vbe_control_info;
    uint32_t vbe_mode_info;
    uint16_t vbe_mode;
    uint16_t vbe_interface_seg;
    uint16_t vbe_interface_off;
    uint16_t vbe_interface_len;

    uint64_t framebuffer_addr;
    uint32_t framebuffer_pitch;
    uint32_t framebuffer_width;
    uint32_t framebuffer_height;
    uint8_t framebuffer_bpp;

    framebuffer_type fb_type;
    union {
        struct {
            uint32_t framebuffer_palette_addr;
            uint16_t framebuffer_palette_num_colors;
        };
        struct {
            uint8_t framebuffer_red_field_position;
            uint8_t framebuffer_red_mask_size;
            uint8_t framebuffer_green_field_position;
            uint8_t framebuffer_green_mask_size;
            uint8_t framebuffer_blue_field_position;
            uint8_t framebuffer_blue_mask_size;
        };
    };
};

template<class OutStr>
void print(OutStr& str, const info_t& info) {
    using tos::print;
    print(str,
          (void*)&info,
          (void*)info.flags,
          reinterpret_cast<void*>(static_cast<uintptr_t>(info.mem_lower)),
          reinterpret_cast<void*>(static_cast<uintptr_t>(info.mem_upper)));
}

inline const info_t* load_info;
} // namespace tos::multiboot