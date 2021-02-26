#pragma once

#include <cstdint>
#include <tos/x86_64/assembly.hpp>

namespace tos::x86_64 {
enum class msrs : uint32_t
{
    ia32_efer = 0xC000'0080,
    star = 0xc000'0081,
    lstar = 0xc000'0082,
    cstar = 0xc000'0083,
    sfmask = 0xc000'0084,
};

inline uint64_t rdmsr(msrs msr) {
    return rdmsr(static_cast<uint32_t>(msr));
}

inline void wrmsr(msrs msr, uint64_t val) {
    wrmsr(static_cast<uint32_t>(msr), val);
}
} // namespace tos::x86_64