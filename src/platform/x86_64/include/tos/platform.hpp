#pragma once

#include <tos/expected.hpp>
#include <tos/function_ref.hpp>
#include <tos/x86_64/exception.hpp>
#include <tos/x86_64/interrupts.hpp>
#include <tos/x86_64/spmanip.hpp>

namespace tos::platform {
using x86_64::disable_interrupts;
using x86_64::enable_interrupts;
using x86_64::interrupts_disabled;

[[noreturn]] void force_reset();

using irq_handler_t = tos::function_ref<void(x86_64::exception_frame*, int)>;

struct irq_errors {};

tos::expected<void, irq_errors> take_irq(int line);
tos::expected<int, irq_errors> allocate_irq();
void free_irq(int line);

void set_irq(int num, irq_handler_t handler);
void set_post_irq(tos::function_ref<void(x86_64::exception_frame*)> handler);
void reset_post_irq();

uintptr_t set_interrupt_rsp(uintptr_t stack_ptr);
} // namespace tos::platform
