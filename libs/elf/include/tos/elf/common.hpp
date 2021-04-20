#pragma once

#include <cstdint>

namespace tos::elf {
enum class elf_class : uint8_t
{
    class32 = 1,
    class64 = 2
};

enum class endian : uint8_t
{
    little = 1,
    big = 2
};

enum class abi : uint8_t
{
    sysv = 0,
    hpux = 1,
    free = 2
};

enum class segment_type : uint32_t
{
    null = 0,
    load = 1,
    dynamic = 2,
    interp = 3,
    note = 4,
    shlib = 5,
    phdr = 6
};

enum class segment_attrs : uint32_t
{
    execute = 1,
    write = 2,
    read = 4
};
} // namespace tos::elf