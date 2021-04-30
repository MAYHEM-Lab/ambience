#include <tos/x86_64/apic.hpp>

namespace tos::x86_64 {
volatile apic_registers& get_apic_registers(uintptr_t apic_at) {
    return *reinterpret_cast<volatile apic_registers*>(apic_at);
}

volatile apic_registers& get_current_apic_registers() {
    return get_apic_registers(get_apic_base_address());
}

uintptr_t get_apic_base_address() {
    auto apic_msr = rdmsr(msrs::ia32_apic_base);
    return apic_msr & ~0xFFF;
}

void set_apic_base_address(uintptr_t addr) {
    auto apic_msr = rdmsr(msrs::ia32_apic_base);
    apic_msr &= 0xFFF;
    apic_msr |= addr & ~0xFFF;
    wrmsr(msrs::ia32_apic_base, apic_msr);
}
} // namespace tos::x86_64