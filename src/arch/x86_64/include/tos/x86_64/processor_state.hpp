#pragma once

#include <cstdint>
#include <csetjmp>

namespace tos::x86_64 {
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
} // namespace tos::x86_64