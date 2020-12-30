#pragma once

namespace tos::x86_64 {
inline void breakpoint() {
    asm volatile("int3");
}

inline uint64_t xgetbv(uint32_t msr) {
    uint64_t msr_value;
    asm volatile("xgetbv" : "=A"(msr_value) : "c"(msr) : "%edx");
    return msr_value;
}

inline void xsetbv(uint32_t msr, uint64_t reg) {
    asm volatile("xsetbv" : : "c"(msr), "A"(reg));
}

inline uint64_t rdmsr(uint32_t msr) {
    uint64_t msr_value;
    asm volatile("rdmsr" : "=A"(msr_value) : "c"(msr));
    return msr_value;
}

inline void wrmsr(uint32_t msr, uint64_t reg) {
    asm volatile("wrmsr" : : "c"(msr), "A"(reg));
}

inline uint32_t read_cr0() {
    uint64_t r;
    __asm__ __volatile__("movq %%cr0, %0" : "=r"(r) : : "memory");
    return r;
}

inline void write_cr0(uint32_t reg) {
    __asm__ __volatile__("mov %0, %%cr0" : : "r"(uint64_t(reg)) : "memory");
}

inline uint32_t read_cr3() {
    uint32_t r;
    __asm__ __volatile__("mov %%cr3, %0" : "=r"(r) : : "memory");
    return r;
}

inline void write_cr3(uint32_t reg) {
    __asm__ __volatile__("mov %0, %%cr3" : : "r"(reg) : "memory");
}

inline uint32_t read_cr4() {
    uint64_t r;
    __asm__ __volatile__("mov %%cr4, %0" : "=r"(r) : : "memory");
    return r;
}

inline void write_cr4(uint32_t reg) {
    __asm__ __volatile__("mov %0, %%cr4" : : "r"(uint64_t(reg)) : "memory");
}
} // namespace tos::x86_64
