#pragma once

#include <csetjmp>
#include <cstdint>

// TODO: Handle pointer mangling here, hosted port is broken otherwise
namespace tos::x86 {
inline uintptr_t get_rip(const jmp_buf& buf) {
    return buf->__jmpbuf[7];
}

inline uintptr_t get_rsp(const jmp_buf& buf) {
    return buf->__jmpbuf[6];
}

inline void set_rip(jmp_buf& buf, uintptr_t val) {
    buf->__jmpbuf[7] = val;
}

inline void set_rsp(jmp_buf& buf, uintptr_t val) {
    buf->__jmpbuf[6] = val;
}
} // namespace tos::x86