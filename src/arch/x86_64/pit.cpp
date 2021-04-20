#include <tos/x86_64/pic.hpp>
#include <tos/x86_64/pit.hpp>
#include <tos/x86_64/port.hpp>

namespace tos::x86_64 {
pit::pit() : m_cb(function_ref<void()>([](void*){})) {
}

void pit::set_frequency(int freq) {
    const uint16_t divisor = (7159090 + 6 / 2) / (6 * freq);

    // The divisor must be sent 8 bits at a time
    const auto low = static_cast<uint8_t>(divisor & 0xFF);
    const auto high = static_cast<uint8_t>((divisor >> 8) & 0xFF);

    // Send the command byte
    port(0x43).outb(0x34);

    // Send the frequency divisor
    port(0x40).outb(low);
    port(0x40).outb(high);
}

void pit::set_callback(function_ref<void()> cb) {
    m_cb = cb;
}

void pit::irq(tos::x86_64::exception_frame*, int) {
    m_cb();
}

void pit::enable() {
    tos::x86_64::port(0x43).outb(0);
    tos::x86_64::port(0x40).outb(0);
    tos::x86_64::port(0x40).outb(0);

    tos::x86_64::pic::enable_irq(0);
}

void pit::disable() {
    tos::x86_64::pic::disable_irq(0);
}
} // namespace tos::x86_64