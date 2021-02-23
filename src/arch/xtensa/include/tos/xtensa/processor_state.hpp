#pragma once

#include <csetjmp>
#include <cstdint>

namespace tos::xtensa {
inline uintptr_t get_rip(const jmp_buf& buf) {
    return buf[0];
}

inline uintptr_t get_rsp(const jmp_buf& buf) {
    return buf[1];
}

inline void set_rip(jmp_buf& buf, uintptr_t val) {
    buf[0] = val;
}

inline void set_rsp(jmp_buf& buf, uintptr_t val) {
    buf[1] = val;
}
} // namespace tos::xtensa