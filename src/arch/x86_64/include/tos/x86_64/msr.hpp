#pragma once

#include <cstdint>
#include <tos/x86_64/assembly.hpp>

namespace tos::x86_64 {
enum class msrs : uint32_t
{
    ia32_efer = 0xc000'0080,

    // Used to store segment selectors for SYSCALL and SYSRET instructions.
    star = 0xc000'0081,

    // Used to store the SYSCALL target.
    lstar = 0xc000'0082,

    // Used to store the SYSCALL target for compatibility mode, not used by tos.
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