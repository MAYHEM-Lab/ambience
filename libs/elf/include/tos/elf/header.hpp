#pragma once

#include <cstdint>
#include <tos/elf/common.hpp>
#include <tos/elf/machine.hpp>
#include <tos/elf/type.hpp>

namespace tos::elf {
template<class T>
struct header;

template<>
struct header<uint64_t> {
    uint8_t magic[4];
    elf_class class_;
    endian enc;
    uint8_t file_version;
    abi os_abi;
    uint8_t abi_ver;
    uint8_t pad[6];
    uint8_t nident;

    type type;
    machine machine;
    uint32_t version;
    uint64_t entry;
    uint64_t pheader_offset;
    uint64_t sheader_offset;
    uint32_t flags;
    uint16_t header_size;
    uint16_t pheader_size;
    uint16_t pheader_num;
    uint16_t shent_size;
    uint16_t shnum;
    uint16_t shstrndx;
};

using elf64_header = header<uint64_t>;
} // namespace tos::elf