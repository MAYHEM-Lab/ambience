#pragma once

#include <cstdint>
#include <tos/function_ref.hpp>

namespace tos::x86_64 {
struct syscall_frame {
    uint64_t rsi;
    uint64_t rdi;
    uint64_t ret_addr;
    uint64_t rflags;
};

using syscall_handler_t = function_ref<void(syscall_frame&)>;

/*
 * Use of SYSCALL/SYSRET instructions require some configuration.
 *
 * This function handles that config. After initialization, `set_syscall_handler` could be
 * used to register the high level handler.
 */
void initialize_syscall_support();

syscall_handler_t set_syscall_handler(syscall_handler_t handler);
} // namespace tos::x86_64