#include <tos/platform.hpp>
#include <tos/x86_64/port.hpp>

namespace tos::platform {
namespace {
void null_irq(x86_64::exception_frame*, int num) {
}

std::array<tos::function_ref<void(x86_64::exception_frame*, int)>, 16> irqs{
    free_function_ref(&null_irq),
    free_function_ref(&null_irq),
    free_function_ref(&null_irq),
    free_function_ref(&null_irq),
    free_function_ref(&null_irq),
    free_function_ref(&null_irq),
    free_function_ref(&null_irq),
    free_function_ref(&null_irq),
    free_function_ref(&null_irq),
    free_function_ref(&null_irq),
    free_function_ref(&null_irq),
    free_function_ref(&null_irq),
    free_function_ref(&null_irq),
    free_function_ref(&null_irq),
    free_function_ref(&null_irq),
    free_function_ref(&null_irq),
};
} // namespace

void set_irq(int num, tos::function_ref<void(x86_64::exception_frame*, int)> handler) {
    irqs[num] = handler;
}
} // namespace tos::platform

extern "C" {
void irq_entry(tos::x86_64::exception_frame* frame, int num) {
    tos::platform::irqs[num - 32](frame, num);
    tos::x86_64::port(0x20).outb(0x20);
    if ((num - 32) >= 8) {
        tos::x86_64::port(0xa0).outb(0x20);
    }
}
}