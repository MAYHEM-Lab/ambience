#pragma once

#include <cstdint>
#include <tos/function_ref.hpp>
#include <tos/x86_64/exception.hpp>

namespace tos::x86_64 {
using syscall_frame = exception_frame;

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