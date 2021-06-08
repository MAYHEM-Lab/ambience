#pragma once

#include <cstdint>

namespace tos::elf {
enum class machine : uint16_t
{
    amd64 = 0x3e,
    bpf = 247
};
}