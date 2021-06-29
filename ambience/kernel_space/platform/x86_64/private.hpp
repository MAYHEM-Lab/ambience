#pragma once

#include <alarm_generated.hpp>
#include <common/alarm.hpp>
#include <common/timer.hpp>
#include <nonstd/variant.hpp>
#include <tos/ae/registry.hpp>
#include <tos/arch.hpp>
#include <tos/expected.hpp>
#include <tos/interrupt_trampoline.hpp>
#include <tos/paging/physical_page_allocator.hpp>
#include <tos/platform.hpp>
#include <tos/preemption.hpp>
#include <tos/x86_64/pit.hpp>

tos::expected<tos::physical_page_allocator*, tos::cur_arch::mmu_errors>
initialize_page_allocator();

void do_acpi_stuff();
void init_pci(tos::physical_page_allocator& palloc, tos::ae::registry_base& registry);
tos::expected<tos::physical_page_allocator*, tos::cur_arch::mmu_errors>
initialize_page_allocator();
void apic_initialize(tos::physical_page_allocator& palloc);
