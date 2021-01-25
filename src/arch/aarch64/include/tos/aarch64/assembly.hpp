#pragma once

#include <cstdint>
#include <tos/barrier.hpp>

namespace tos::aarch64 {
inline void nop() {
    asm volatile("nop");
}

inline void breakpoint() {
    asm volatile("brk #0");
}

inline void isb() {
    tos::detail::memory_barrier();
    asm volatile("isb");
    tos::detail::memory_barrier();
}

inline void dsb() {
    tos::detail::memory_barrier();
    asm volatile("dsb sy");
    tos::detail::memory_barrier();
}

// https://developer.arm.com/docs/ddi0595/d/aarch64-system-registers/currentel
inline uint32_t get_execution_level() {
    uint32_t el;
    asm volatile("mrs %0, CurrentEL" : "=r"(el));
    return (el >> 2) & 0b11;
}

// See: https://developer.arm.com/docs/ddi0595/d/aarch64-system-registers/ttbr0_el1
inline void set_ttbr0_el1(void* ptr) {
    tos::detail::memory_barrier();
    asm volatile("msr ttbr0_el1, %0" : : "r"(ptr));
    tos::detail::memory_barrier();
}

// See: https://developer.arm.com/docs/ddi0595/d/aarch64-system-registers/ttbr0_el1
inline uintptr_t get_ttbr0_el1() {
    uint64_t r;
    asm volatile("mrs %0, ttbr0_el1" : "=r"(r));
    return r;
}

// See: https://developer.arm.com/docs/ddi0595/d/aarch64-system-registers/ttbr1_el1
inline void set_ttbr1_el1(void* ptr) {
    tos::detail::memory_barrier();
    asm volatile("msr ttbr1_el1, %0" : : "r"(ptr));
    tos::detail::memory_barrier();
}

enum class pa_ranges : uint64_t
{
    bits_32 = 0b0000,
    bits_36 = 0b0001,
    bits_40 = 0b0010,
    bits_42 = 0b0011,
    bits_44 = 0b0100,
    bits_48 = 0b0101,
    bits_52 = 0b0110
};

struct id_aa64mmfr0_t {
    union {
        uint64_t raw;
        struct {
            pa_ranges PARange : 4;
            uint64_t ASIDBits : 4;
            uint64_t BigEnd : 4;
            uint64_t SNSMem : 4;
            uint64_t BigEndEL0 : 4;
            uint64_t TGran16 : 4;
            uint64_t TGran64 : 4;
            uint64_t TGran4 : 4;
        };
    };
};

// See: https://developer.arm.com/docs/ddi0595/d/aarch64-system-registers/id_aa64mmfr0_el1
inline id_aa64mmfr0_t get_id_aa64mmfr0_el1() {
    id_aa64mmfr0_t r;
    asm volatile("mrs %0, id_aa64mmfr0_el1" : "=r"(r));
    return r;
}

// See: https://developer.arm.com/docs/ddi0595/b/aarch64-system-registers/mair_el1
inline void set_mair_el1(uint64_t reg) {
    asm volatile("msr mair_el1, %0" : : "r"(reg));
}

// See: https://developer.arm.com/docs/ddi0595/b/aarch64-system-registers/tcr_el1
inline void set_tcr_el1(uint64_t reg) {
    asm volatile("msr tcr_el1, %0" : : "r"(reg));
}

// See: https://developer.arm.com/docs/ddi0595/b/aarch64-system-registers/sctlr_el1
inline void set_sctlr_el1(uint64_t reg) {
    asm volatile("msr sctlr_el1, %0" : : "r"(reg));
}

// See: https://developer.arm.com/docs/ddi0595/b/aarch64-system-registers/sctlr_el1
inline uint64_t get_sctlr_el1() {
    uint64_t r;
    asm volatile("mrs %0, sctlr_el1" : "=r"(r));
    return r;
}

inline void udf() {
    // For some reason GNU assembler does not implement the UDF instruction.
    // Its encoding is 0x00xx, and we pick 0x0000 here.
    asm volatile (".word 0");
}

inline void set_spsr_el1(uint64_t reg) {
    asm volatile("msr spsr_el1, %0" : : "r"(reg));
}

inline void set_elr_el1(uintptr_t ret_addr) {
    asm volatile("msr elr_el1, %0" : : "r"(ret_addr));
}

inline void set_sp_el0(uintptr_t stack_ptr) {
    asm volatile("msr sp_el0, %0" : : "r"(stack_ptr));
}

inline uint64_t get_esr_l1() {
    uint64_t r;
    asm volatile("mrs %0, esr_el1" : "=r"(r));
    return r;
}

inline uint64_t get_elr_l1() {
    uint64_t r;
    asm volatile("mrs %0, elr_el1" : "=r"(r));
    return r;
}

inline uint64_t get_spsr_l1() {
    uint64_t r;
    asm volatile("mrs %0, spsr_el1" : "=r"(r));
    return r;
}

inline uint64_t get_far_l1() {
    uint64_t r;
    asm volatile("mrs %0, far_el1" : "=r"(r));
    return r;
}

inline uintptr_t get_sp_el1() {
    uint64_t r;
    asm volatile("mrs %0, sp_el1" : "=r"(r));
    return r;
}

inline void eret() {
    asm volatile("eret");
}

inline void svc1() {
    asm volatile ("svc #1");
}

inline void svc2() {
    asm volatile ("svc #2");
}

inline void wfi() {
    asm volatile ("wfi");
}

// This intrinsic is used to invoke the semihosting host. See the semihosting docs for
// more details.
inline void hlt_0xf000() {
    asm volatile("hlt #0xF000");
}

inline uint64_t get_cntpct_el0() {
    uint64_t r;
    asm volatile("mrs %0, cntpct_el0" : "=r"(r));
    return r;
}

inline uint64_t get_cntfrq_el0() {
    uint64_t r;
    asm volatile("mrs %0, cntfrq_el0" : "=r"(r));
    return r;
}

inline uint64_t get_cntp_ctl_el0() {
    uint64_t r;
    asm volatile("mrs %0, cntp_ctl_el0" : "=r"(r));
    return r;
}

inline void set_cntp_ctl_el0(uint64_t val) {
    asm volatile("msr cntp_ctl_el0, %0" : : "r"(val));
}

inline uint64_t get_cntp_cval_el0() {
    uint64_t r;
    asm volatile("mrs %0, cntp_cval_el0" : "=r"(r));
    return r;
}

inline void set_cntp_cval_el0(uint64_t val) {
    asm volatile("msr cntp_cval_el0, %0" : : "r"(val));
}

inline uint64_t get_cntp_tval_el0() {
    uint64_t r;
    asm volatile("mrs %0, cntp_tval_el0" : "=r"(r));
    return r;
}

inline void set_cntp_tval_el0(uint64_t val) {
    asm volatile("msr cntp_tval_el0, %0" : : "r"(val));
}
} // namespace tos::aarch64