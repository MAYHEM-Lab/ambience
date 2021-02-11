#pragma once

#include <csetjmp>
#include <cstdint>

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
} // namespace tos::arm