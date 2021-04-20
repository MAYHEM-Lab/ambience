#include <tos/platform.hpp>
#include <tos/x86_64/port.hpp>

namespace tos::platform {
namespace {
void null_irq(x86_64::exception_frame*, int num) {
}

void null_post_irq(x86_64::exception_frame*) {
}

tos::function_ref<void(x86_64::exception_frame*)> post_irq{
    free_function_ref(&null_post_irq)};

std::array<irq_handler_t, 16> irqs{
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

void set_irq(int num, irq_handler_t handler) {
    irqs[num] = handler;
}

void set_post_irq(tos::function_ref<void(x86_64::exception_frame*)> handler) {
    post_irq = handler;
}

void reset_post_irq() {
    post_irq = free_function_ref(null_post_irq);
}
} // namespace tos::platform

extern "C" {
[[gnu::used]] void irq_entry(tos::x86_64::exception_frame* frame, int num) {
    tos::platform::irqs[num - 32](frame, num);
    tos::x86_64::port(0x20).outb(0x20);
    if ((num - 32) >= 8) {
        tos::x86_64::port(0xa0).outb(0x20);
    }
    tos::platform::post_irq(frame);
}
}