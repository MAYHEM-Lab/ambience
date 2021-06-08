#pragma once

#include <cstdint>
#include <string_view>
#include <tos/ebpf/opcode.hpp>
#include <tos/function_ref.hpp>
#include <tos/span.hpp>
#include <vector>

namespace tos::ebpf {
using external_function =
    function_ref<uint64_t(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t)>;

struct execution_context {
    span<const instruction> instructions;
    span<uint8_t> memory;
    bool bounds_check_enabled = false;
    span<const external_function> ext_funcs{nullptr};
    span<const std::string_view> ext_func_names{nullptr};
};

uint64_t execute(const execution_context& ctx);
} // namespace tos::ebpf