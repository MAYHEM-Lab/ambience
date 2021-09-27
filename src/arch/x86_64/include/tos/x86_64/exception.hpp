#pragma once

#include <cstdint>
#include <nonstd/variant.hpp>

namespace tos::x86_64 {
struct [[gnu::packed]] exception_frame {
    union {
        struct [[gnu::packed]] {
            uint64_t r15;
            uint64_t r14;
            uint64_t r13;
            uint64_t r12;
            uint64_t r11;
            uint64_t r10;
            uint64_t r9;
            uint64_t r8;
            uint64_t rbp;
            uint64_t rdi;
            uint64_t rsi;
            uint64_t rdx;
            uint64_t rcx;
            uint64_t rbx;
            uint64_t rax;
        };
        uint64_t gpr[15];
    };
    uint64_t error_code;
    uint64_t rip;
    uint64_t cs;
    uint64_t rflags;
    uint64_t rsp;
    uint64_t ss;
};


struct base_fault {
    uintptr_t instr_address;
};

struct undefined_instruction : base_fault {
};

struct page_fault : base_fault {
    uintptr_t fault_address;
    bool access_type;
};

using fault_variant = mpark::variant<undefined_instruction, page_fault>;
namespace global {
extern exception_frame* cur_exception_frame;
}
} // namespace tos::x86_64