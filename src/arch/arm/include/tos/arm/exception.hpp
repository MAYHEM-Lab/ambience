#pragma once

#include <cstdint>
#include <tos/function_ref.hpp>
#include <variant>

namespace tos::arm::exception {
struct [[gnu::packed]] stack_frame_t {
    uint32_t r0;
    uint32_t r1;
    uint32_t r2;
    uint32_t r3;
    uint32_t r12;
    uint32_t lr;
    uint32_t return_address;
    uint32_t xpsr;
};

struct unknown_fault {
    uintptr_t instr_address = 0;
};

struct div_by_zero_fault {
    uintptr_t instr_address;
};

struct undefined_instruction_fault {
    uintptr_t instr_address;
};

struct memory_fault {
    uintptr_t instr_address;
    uintptr_t data_address;
};

struct bus_fault_t {
    uintptr_t instr_address;
    uintptr_t fault_address;
    bool precise;
};

using fault_variant = std::variant<unknown_fault,
                                   undefined_instruction_fault,
                                   div_by_zero_fault,
                                   memory_fault,
                                   bus_fault_t>;

void set_general_fault_handler(tos::function_ref<bool(const fault_variant&)> handler);

void hard_fault();
void mem_fault();
void bus_fault();
void usage_fault();
} // namespace tos::arm::exception