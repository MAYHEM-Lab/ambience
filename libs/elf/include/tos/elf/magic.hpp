#pragma once

#include <cstdint>

namespace tos::elf {
inline constexpr uint8_t magic[] = {
    0x7f, 0x45, 0x4c, 0x46 // \x7fELF
};
}