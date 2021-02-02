#pragma once

#include <tos/function_ref.hpp>
#include <tos/x86_64/exception.hpp>
#include <tos/x86_64/interrupts.hpp>
#include <tos/x86_64/spmanip.hpp>

namespace tos::platform {
using x86_64::disable_interrupts;
using x86_64::enable_interrupts;
using x86_64::interrupts_disabled;

[[noreturn]] void force_reset();

void set_irq(int num, tos::function_ref<void(x86_64::exception_frame*, int)> handler);
} // namespace tos::platform
