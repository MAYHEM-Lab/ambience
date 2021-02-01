#pragma once

#include <cstdint>
#include <tos/elf/common.hpp>

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
};

using elf64_program_header = program_header<uint64_t>;

static_assert(sizeof(elf64_program_header) == 56);
} // namespace tos::elf