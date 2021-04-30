#pragma once

#include <cstddef>
#include <cstdint>
#include <tos/x86_64/msr.hpp>

namespace tos::x86_64 {
struct apic_registers {
    uint64_t __pad1[4]; // 32 bytes
    alignas(16) uint32_t id;
    alignas(16) uint32_t version;
    uint64_t __pad2[8];
    alignas(16) uint32_t tpr;
    alignas(16) uint32_t apr;
    alignas(16) uint32_t ppr;
    alignas(16) uint32_t eoi;
    alignas(16) uint32_t rrr;
    alignas(16) uint32_t ldr;
    alignas(16) uint32_t dfr;
    alignas(16) uint32_t sivr;
};

static_assert(offsetof(apic_registers, id) == 0x20);
static_assert(offsetof(apic_registers, version) == 0x30);
static_assert(offsetof(apic_registers, tpr) == 0x80);
static_assert(offsetof(apic_registers, eoi) == 0xB0);
static_assert(offsetof(apic_registers, sivr) == 0xF0);

volatile apic_registers& get_apic_registers(uintptr_t apic_at);
volatile apic_registers& get_current_apic_registers();

uintptr_t get_apic_base_address();
void set_apic_base_address(uintptr_t);

inline bool apic_enabled = false;

struct apic {
    apic(volatile apic_registers& regs)
        : m_regs{&regs} {
    }

    void enable() {
        m_regs->sivr = m_regs->sivr | (1 << 8);
        apic_enabled = true;
    }

    volatile apic_registers* m_regs;
};

inline uint32_t ioapic_read(void* ioapic_base, uint32_t reg) {
    auto ioapic = static_cast<volatile uint32_t*>(ioapic_base);
    // IOREGSEL
    ioapic[0] = (reg & 0xff);
    return ioapic[4];
}

inline void ioapic_write(void* ioapic_base, uint32_t reg, uint32_t value) {
    auto ioapic = static_cast<volatile uint32_t*>(ioapic_base);
    ioapic[0] = (reg & 0xff);
    ioapic[4] = value;
}

inline void ioapic_set_irq(uint8_t irq, uint64_t apic_id, uint8_t vector) {
    const uint32_t low_index = 0x10 + irq * 2;
    const uint32_t high_index = 0x10 + irq * 2 + 1;

    uint32_t high = ioapic_read((void*)0xfec00000, high_index);
    high = 0;
    // set APIC ID
    high &= ~0xff000000;
    high |= apic_id << 24;
    ioapic_write((void*)0xfec00000, high_index, high);

    uint32_t low = ioapic_read((void*)0xfec00000, low_index);
    low = 0;

    // unmask the IRQ
    low &= ~(1 << 16);

    // set to physical delivery mode
    low &= ~(1 << 11);

    // set to fixed delivery mode
    low &= ~0x700;

    // set delivery vector
    low &= ~0xff;
    low |= vector;

    ioapic_write((void*)0xfec00000, low_index, low);
}
} // namespace tos::x86_64