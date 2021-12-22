#pragma once

#include <cstdint>
#include <tos/x86_64/assembly.hpp>

namespace tos::x86_64 {
enum class msrs : uint32_t
{
    ia32_apic_base = 0x1b,

    ia32_efer = 0xc000'0080,

    // Used to store segment selectors for SYSCALL and SYSRET instructions.
    star = 0xc000'0081,

    // Used to store the SYSCALL target.
    lstar = 0xc000'0082,

    // Used to store the SYSCALL target for compatibility mode, not used by tos.
    cstar = 0xc000'0083,
    sfmask = 0xc000'0084,

    perf_evt_sel0_l = 0xC001'0000,
    perf_evt_sel1_l = 0xC001'0001,
    perf_evt_sel2_l = 0xC001'0002,
    perf_evt_sel3_l = 0xC001'0003,

    perf_ctr0_l = 0xC001'0004,
    perf_ctr1_l = 0xC001'0005,
    perf_ctr2_l = 0xC001'0006,
    perf_ctr3_l = 0xC001'0007,

    perf_ctl0= 0xC001'0200,
    perf_ctl1= 0xC001'0202,
    perf_ctl2= 0xC001'0204,
    perf_ctl3= 0xC001'0206,
    perf_ctl4= 0xC001'0208,
    perf_ctl5= 0xC001'020A,

    perf_ctr0= 0xC001'0201,
    perf_ctr1= 0xC001'0203,
    perf_ctr2= 0xC001'0204,
    perf_ctr3= 0xC001'0207,
    perf_ctr4= 0xC001'0209,
    perf_ctr5= 0xC001'020B,
};

inline uint64_t rdmsr(msrs msr) {
    return rdmsr(static_cast<uint32_t>(msr));
}

inline void wrmsr(msrs msr, uint64_t val) {
    wrmsr(static_cast<uint32_t>(msr), val);
}
} // namespace tos::x86_64