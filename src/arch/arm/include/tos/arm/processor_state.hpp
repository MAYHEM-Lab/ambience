#pragma once

#include <csetjmp>
#include <cstdint>

extern "C" void tos_call_trampoline();
namespace tos::arm {
inline uintptr_t get_rip(const jmp_buf& buf) {
    return buf[9];
}

inline uintptr_t get_rsp(const jmp_buf& buf) {
    return buf[8];
}

inline void set_rip(jmp_buf& buf, uintptr_t val) {
    buf[9] = val;
}

inline void set_rsp(jmp_buf& buf, uintptr_t val) {
    buf[8] = val;
}

inline void make_call(jmp_buf& buf, void(*entry)(void*), void* ptr) {
    buf[0] = reinterpret_cast<uintptr_t>(entry);
    buf[1] = reinterpret_cast<uintptr_t>(ptr);
    set_rip(buf, reinterpret_cast<uintptr_t>(&tos_call_trampoline));
}
} // namespace tos::arm