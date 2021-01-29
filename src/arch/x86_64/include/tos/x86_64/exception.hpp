#pragma once

#include <cstdint>

namespace tos::x86_64 {
struct [[gnu::packed]] exception_frame {
    uint64_t gpr[15];
    uint64_t error_code;
    uint64_t rip;
    uint64_t cs;
    uint64_t rflags;
    uint64_t rsp;
    uint64_t ss;
};
} // namespace tos::x86_64