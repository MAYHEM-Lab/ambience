#pragma once

#include <cstdint>
#include <csetjmp>

extern "C" void tos_call_trampoline();
namespace tos::x86_64 {
 /*
 **   rbx rbp r12 r13 r14 r15 rsp rip
 **   0   8   16  24  32  40  48  56
 */

inline uintptr_t get_rip(const jmp_buf& buf) {
    return buf[7];
}

inline uintptr_t get_rsp(const jmp_buf& buf) {
    return buf[6];
}

inline void set_rip(jmp_buf& buf, uintptr_t val) {
    buf[7] = val;
}

inline void set_rsp(jmp_buf& buf, uintptr_t val) {
    buf[6] = val;
}

inline void set_r15(jmp_buf& buf, uintptr_t val) {
    buf[5] = val;
}

inline void set_r14(jmp_buf& buf, uintptr_t val) {
    buf[4] = val;
}

inline void make_call(jmp_buf& buf, void(*fn)(void*), void* param) {
    // r14 has fn
    // r15 has param
    set_r14(buf, reinterpret_cast<uintptr_t>(fn));
    set_r15(buf, reinterpret_cast<uintptr_t>(param));
    set_rip(buf, reinterpret_cast<uintptr_t>(&tos_call_trampoline));
}
} // namespace tos::x86_64