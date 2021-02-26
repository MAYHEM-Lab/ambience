#pragma once

#include <cstdint>
#include <tos/compiler.hpp>

namespace tos::x86_64 {
struct PACKED tss {
    uint32_t reserved_;
    // Rings 0 through 2
    uint64_t rsp[3];
    uint32_t reserved_2[2];
    uint64_t ist[7];
    uint32_t reserved_3[2];
    uint16_t reserved_4;
    uint16_t iopb_offset;
};
}