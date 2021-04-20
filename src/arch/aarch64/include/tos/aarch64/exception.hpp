#pragma once

#include <cstdint>
#include <tos/function_ref.hpp>
#include <variant>

namespace tos::aarch64::exception {
struct stack_frame_t {
    uint64_t gpr[31];
    uint64_t sp;
    uint64_t lr;
    uint64_t spsr;
};

enum class exception_classes
{
    undefined = 0,
    SVC32 = 0b010001,
    SVC64 = 0b010101,
    instruction_abort_lower_level = 0b100000,
    instruction_abort = 0b100001,
    data_abort_lower_level = 0b100100,
    data_abort = 0b100101,
};

struct fault_t {
    uintptr_t return_address;
};

struct instruction_abort : fault_t {
    uintptr_t instr_addr;
    uint8_t ifsc;
};

enum class access_size : uint8_t
{
    byte,
    halfword,
    word,
    doubleword
};

struct data_abort : fault_t {
    uintptr_t data_addr;
    access_size size;
    bool sign_extend;
    uint64_t iss;
};

struct undefined_instruction : fault_t {};

struct svc_exception : fault_t {
    uint32_t svnum;
};

struct unknown_fault : fault_t {};

using fault_variant = std::variant<unknown_fault,
                                   instruction_abort,
                                   data_abort,
                                   undefined_instruction,
                                   svc_exception>;

using svc_handler_t = tos::function_ref<void(int, stack_frame_t&)>;
using fault_handler_t = tos::function_ref<void(const fault_variant&, stack_frame_t&)>;

svc_handler_t set_svc_handler(svc_handler_t handler);
fault_handler_t set_fault_handler(fault_handler_t handler);
} // namespace tos::aarch64::exception