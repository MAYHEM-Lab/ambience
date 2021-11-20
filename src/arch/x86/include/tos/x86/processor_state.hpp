#pragma once

#include <csetjmp>
#include <cstdint>

#define TOS_ARCH_HAS_PROC_STATE
extern "C" void tos_call_trampoline();
namespace tos::x86 {
using proc_state_t = uint64_t[8];
 /*
 **   rbx rbp r12 r13 r14 r15 rsp rip
 **   0   8   16  24  32  40  48  56
 */

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

inline void set_r15(proc_state_t& buf, uintptr_t val) {
    buf[5] = val;
}

inline void set_r14(proc_state_t& buf, uintptr_t val) {
    buf[4] = val;
}

inline void make_call(proc_state_t& buf, void(*fn)(void*), void* param) {
    // r14 has fn
    // r15 has param
    set_r14(buf, reinterpret_cast<uintptr_t>(fn));
    set_r15(buf, reinterpret_cast<uintptr_t>(param));
    set_rip(buf, reinterpret_cast<uintptr_t>(&tos_call_trampoline));
}
} // namespace tos::x86

extern "C" [[gnu::returns_twice]] uint64_t tos_setjmp(tos::x86::proc_state_t);

extern "C" [[noreturn]] uint64_t tos_longjmp(tos::x86::proc_state_t, uint64_t);