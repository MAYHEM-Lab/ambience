#include <tos/debug/log.hpp>
#include <tos/platform.hpp>
#include <tos/x86_64/apic.hpp>
#include <tos/x86_64/port.hpp>

namespace tos::platform {
namespace {
void null_irq(x86_64::exception_frame*, int num) {
    tos::debug::warn("Unhandled interrupt!", num);
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

uint16_t allocated_irqs = 0;
} // namespace

tos::expected<void, irq_errors> take_irq(int line) {
    if (allocated_irqs & (1 << line)) {
        return unexpected(irq_errors{});
    }
    allocated_irqs |= (1 << line);
    return {};
}

tos::expected<int, irq_errors> allocate_irq() {
    for (int i = 0; i < 16; ++i) {
        if (take_irq(i)) {
            return i;
        }
    }
    return unexpected(irq_errors{});
}

void free_irq(int line) {
    Assert(allocated_irqs & (1 << line));
    allocated_irqs &= ~(1 << line);
}

void set_irq(int num, irq_handler_t handler) {
    Assert(allocated_irqs & (1 << num));
    irqs[num] = handler;
}

void set_post_irq(tos::function_ref<void(x86_64::exception_frame*)> handler) {
    post_irq = handler;
}

void reset_post_irq() {
    post_irq = free_function_ref(null_post_irq);
}
} // namespace tos::platform

bool enable_rip_rec = false;
NO_ZERO uintptr_t rip_buffer[128 * 1024];
int rip_idx = 0;

extern "C" {
[[gnu::used]] void irq_entry(tos::x86_64::exception_frame* frame, int num) {
    tos::x86_64::global::cur_exception_frame = frame;

    // IRQ 12 is used for ODI, we don't care
    if (num != 44 && enable_rip_rec) {
        rip_buffer[rip_idx] = static_cast<uintptr_t>(frame->rip);
        rip_idx = (rip_idx + 1) % std::size(rip_buffer);
    }

    tos::platform::irqs[num - 32](frame, num);
    if (tos::x86_64::apic_enabled) {
        tos::x86_64::get_current_apic_registers().eoi = 0;
    } else {
        tos::x86_64::port(0x20).outb(0x20);
        if ((num - 32) >= 8) {
            tos::x86_64::port(0xa0).outb(0x20);
        }
    }
    tos::platform::post_irq(frame);
}
}