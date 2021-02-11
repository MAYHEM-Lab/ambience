#pragma once

#include <csetjmp>
#include <cstdint>

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
} // namespace tos::aarch64