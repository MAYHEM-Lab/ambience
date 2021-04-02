#pragma once

#include <cpuid.h>
#include <cstdint>
#include <tos/compiler.hpp>

namespace tos::x86_64 {
inline void breakpoint() {
    asm volatile("int3");
}

inline void hlt() {
    asm volatile("hlt");
}

inline uint64_t read_rcx() {
    uint64_t res;
    asm volatile("movq %%rcx, %0" : "=r"(res));
    return res;
}

inline uint64_t read_flags() {
    uint64_t flags;
    asm volatile("pushf\n"
                 "pop %0"
                 : "=g"(flags));
    return flags;
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
    uint32_t low, high;
    asm volatile("rdmsr" : "=a"(low), "=d"(high) : "c"(msr));
    return ((uint64_t)high << 32) | low;
}

inline void wrmsr(uint32_t msr, uint64_t reg) {
    uint32_t low = reg & 0xFFFFFFFF;
    uint32_t high = reg >> 32;
    asm volatile("wrmsr" : : "c"(msr), "a"(low), "d"(high));
}

inline uint32_t read_cr0() {
    uint64_t r;
    __asm__ __volatile__("movq %%cr0, %0" : "=r"(r) : : "memory");
    return r;
}

inline void write_cr0(uint32_t reg) {
    __asm__ __volatile__("mov %0, %%cr0" : : "r"(uint64_t(reg)) : "memory");
}

inline uint64_t read_cr2() {
    uint64_t r;
    __asm__ __volatile__("mov %%cr2, %0" : "=r"(r) : : "memory");
    return r;
}

inline uint64_t read_cr3() {
    uint64_t r;
    __asm__ __volatile__("mov %%cr3, %0" : "=r"(r) : : "memory");
    return r;
}

inline void write_cr3(uint64_t reg) {
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

/**
 * Jumps to the given address in ring 3.
 * The stack pointer is not adjusted in any way.
 * This means that using this instruction in a C++ function will **not** pop any pushed
 * registers etc.
 * Only meant to be used while initializing the user space, not for returning from
 * syscalls. For those, just use a regular return, restore the state in assembly and then
 * use sysret. In other words:
 *
 *      raw_syscall_entry:
 *           pushq %rcx
 *           pushq %rsp
 *           callq syscall_entry
 *           popq %rsp
 *           popq %rcx
 *           sysretq
 *
 * @param addr instruction pointer to return to.
 */
[[noreturn]] inline void sysret(void* addr) {
    asm volatile("sysretq" : : "c"(addr));
    TOS_UNREACHABLE();
}

inline void syscall(uint64_t rdi, uint64_t rsi) {
    asm volatile("syscall" : : "D"(rdi), "S"(rsi) : "memory");
}

inline void int0x2c() {
    asm volatile("int $0x2c" : : : "memory");
}

inline void invlpg(uintptr_t addr) {
    asm volatile("invlpg (%0)" : : "r"(addr) : "memory");
}
} // namespace tos::x86_64
