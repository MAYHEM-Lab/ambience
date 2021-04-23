#include <tos/x86_64/pic.hpp>
#include <tos/x86_64/pit.hpp>
#include <tos/x86_64/port.hpp>
#include <tos/debug/log.hpp>

namespace tos::x86_64 {
void pit::set_frequency(int freq) {
    // PIT ticks at a fixed 1.193182 MHz
    // 1193182 Hz
    m_freq = freq;
    const uint16_t divisor = 1'193'182 / freq;

    // The divisor must be sent 8 bits at a time
    const auto low = static_cast<uint8_t>(divisor & 0xFF);
    const auto high = static_cast<uint8_t>((divisor >> 8) & 0xFF);

    // Send the command byte
    port(0x43).outb(0x34);

    // Send the frequency divisor
    port(0x40).outb(low);
    port(0x40).outb(high);
}

void pit::enable() {
    tos::x86_64::pic::enable_irq(0);
}

void pit::disable() {
    tos::x86_64::pic::disable_irq(0);
}

uint32_t pit::get_period() const {
    return 0;
}
uint32_t pit::get_counter() const {
    tos::x86_64::port(0x43).outb(0);
    auto lo = tos::x86_64::port(0x40).inb();
    auto hi = tos::x86_64::port(0x40).inb();
    return hi << 8 | lo;
}
} // namespace tos::x86_64