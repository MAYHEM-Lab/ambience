#pragma once

#include <cstdint>
#include <tos/elf/common.hpp>
#include <tos/flags.hpp>
#include <tos/memory.hpp>

namespace tos::elf {
template<class T>
struct program_header;

template<>
struct program_header<uint64_t> {
    segment_type type;
    segment_attrs attrs;
    alignas(8) uint64_t file_offset;
    alignas(8) uint64_t virt_address;
    alignas(8) uint64_t p_address;
    alignas(8) uint64_t file_size;
    alignas(8) uint64_t virt_size;
    alignas(8) uint64_t alignment;

    constexpr tos::virtual_address virtual_address() const {
        return tos::virtual_address(virt_address);
    }

    constexpr tos::virtual_range virtual_range() const {
        return tos::virtual_range{virtual_address(), static_cast<ptrdiff_t>(virt_size)};
    }

    constexpr tos::virtual_segment virtual_segment() const {
        auto perms = permissions::none;
        if (util::is_flag_set(attrs, segment_attrs::read)) {
            perms = util::set_flag(perms, permissions::read);
        }
        if (util::is_flag_set(attrs, segment_attrs::write)) {
            perms = util::set_flag(perms, permissions::write);
        }
        if (util::is_flag_set(attrs, segment_attrs::execute)) {
            perms = util::set_flag(perms, permissions::execute);
        }
        return tos::virtual_segment{virtual_range(), perms};
    }
};

using elf64_program_header = program_header<uint64_t>;

static_assert(sizeof(elf64_program_header) == 56);
} // namespace tos::elf