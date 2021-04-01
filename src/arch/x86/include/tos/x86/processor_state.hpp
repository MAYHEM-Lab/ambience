#pragma once

#include <csetjmp>
#include <cstdint>

#define TOS_ARCH_HAS_PROC_STATE

// TODO: Handle pointer mangling here, hosted port is broken otherwise
namespace tos::x86 {
using proc_state_t = uint64_t[8];

inline uintptr_t get_rip(const proc_state_t& buf) {
    return buf[7];
}

inline uintptr_t get_rsp(const proc_state_t& buf) {
    return buf[6];
}

inline void set_rip(proc_state_t& buf, uintptr_t val) {
    buf[7] = val;
}

inline void set_rsp(proc_state_t& buf, uintptr_t val) {
    buf[6] = val;
}
} // namespace tos::x86

extern "C" [[gnu::returns_twice]] uint64_t tos_setjmp(tos::x86::proc_state_t);

extern "C" [[noreturn]] uint64_t tos_longjmp(tos::x86::proc_state_t, uint64_t);