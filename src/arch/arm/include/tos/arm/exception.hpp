#pragma once

#include <variant>
#include <cstdint>
#include <tos/function_ref.hpp>

namespace tos::arm::exception {
struct unknown_fault {};

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

using fault_variant = std::
    variant<unknown_fault, undefined_instruction_fault, div_by_zero_fault, memory_fault>;

void set_general_fault_handler(tos::function_ref<bool(const fault_variant&)> handler);

void hard_fault();
void mem_fault();
void usage_fault();
} // namespace tos::arm::exception