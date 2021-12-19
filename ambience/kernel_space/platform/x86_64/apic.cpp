#include "private.hpp"
#include "tos/memory.hpp"
#include <tos/x86_64/apic.hpp>
#include <tos/x86_64/pic.hpp>

void apic_initialize(tos::physical_page_allocator& palloc) {
    auto apic_base = tos::x86_64::get_apic_base_address();
    LOG((void*)apic_base, (void*)tos::x86_64::rdmsr(tos::x86_64::msrs::ia32_apic_base));
    auto& table = tos::cur_arch::get_current_translation_table();
    auto seg = tos::physical_segment{
        .range = {tos::physical_address(apic_base), tos::cur_arch::page_size_bytes},
        .perms = tos::permissions::read_write};
    tos::ensure(tos::x86_64::map_region(table,
                                        identity_map(seg),
                                        tos::user_accessible::no,
                                        tos::memory_types::device,
                                        &palloc,
                                        tos::physical_address(apic_base)));

    using namespace tos::address_literals;

    // IOAPIC registers
    seg = tos::physical_segment{
        .range = {0xfec00000_physical, tos::cur_arch::page_size_bytes},
        .perms = tos::permissions::read_write};
    tos::ensure(tos::x86_64::map_region(table,
                                        identity_map(seg),
                                        tos::user_accessible::no,
                                        tos::memory_types::device,
                                        &palloc,
                                        0xfec00000_physical));

    auto& apic_regs = tos::x86_64::get_apic_registers(apic_base);
    LOG((void*)(uintptr_t)apic_regs.id, (void*)(uintptr_t)apic_regs.version);

    do_acpi_stuff();

    tos::x86_64::pic::disable();
    tos::x86_64::apic apic(apic_regs);
    apic.enable();
    LOG(apic_regs.tpr);
    apic_regs.tpr = 0;

    auto IOAPICID = tos::x86_64::ioapic_read((void*)0xfec00000, 0);
    LOG(IOAPICID);
    auto IOAPICVER = tos::x86_64::ioapic_read((void*)0xfec00000, 1);
    LOG((void*)(uintptr_t)IOAPICVER);

    // Timer
    tos::x86_64::ioapic_set_irq(2, apic_regs.id, 32 + 0);

    // Keyboard
    tos::x86_64::ioapic_set_irq(1, apic_regs.id, 32 + 1);
}