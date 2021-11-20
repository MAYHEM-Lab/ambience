#pragma once

#include <csetjmp>
#include <cstdint>

extern "C" void tos_call_trampoline();
namespace tos::aarch64 {
inline uintptr_t get_rip(const jmp_buf& buf) {
    return buf[11];
}

inline uintptr_t get_rsp(const jmp_buf& buf) {
    return buf[12];
}

inline void set_rip(jmp_buf& buf, uintptr_t val) {
    buf[11] = val;
}

inline void set_rsp(jmp_buf& buf, uintptr_t val) {
    buf[12] = val;
}

inline void make_call(jmp_buf& buf, void(*entry)(void*), void* ptr) {
    buf[0] = reinterpret_cast<uintptr_t>(entry);
    buf[1] = reinterpret_cast<uintptr_t>(ptr);
    set_rip(buf, reinterpret_cast<uintptr_t>(&tos_call_trampoline));
}
} // namespace tos::aarch64