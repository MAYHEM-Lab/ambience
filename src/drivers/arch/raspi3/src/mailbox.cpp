//
// Created by fatih on 3/10/20.
//

#include <arch/mailbox.hpp>
#include <tos/ft.hpp>
#include <tos/soc/bcm2837.hpp>

namespace tos::raspi3 {
using bcm2837::VIDEOCORE_MBOX;
void mailbox_channel::write(uint32_t word) {
    tos::lock_guard g{m_prot};
    while (VIDEOCORE_MBOX->status_full()) {
        tos::this_thread::yield();
    }

    uint32_t to_write =
        (word << 4U) | static_cast<uint32_t>(m_channel);
    VIDEOCORE_MBOX->write = to_write;
}

uint32_t mailbox_channel::read() {
    tos::lock_guard g{m_prot};
    while (VIDEOCORE_MBOX->status_empty()) {
        tos::this_thread::yield();
    }

    uint32_t read_word = VIDEOCORE_MBOX->read;
    auto chan = (read_word & 0x00'00'00'0FU);
    if (chan != static_cast<uint32_t>(m_channel)) {
        // not good
        return read();
    }
    return read_word >> 4U;
}

bool property_channel::transaction(span<uint32_t> buffer) {
    uint64_t addr_to_write = reinterpret_cast<uint64_t>(buffer.data());
    if ((addr_to_write & ~(0xFF'FF'FF'F0)) != 0) {
        // error
        tos::debug::panic("bad property channel buffer address!");
    }
    write(addr_to_write >> 4U);
    read(); // response is written to buffer
    return buffer[1] == 0x80000000;
}
} // namespace tos::raspi3