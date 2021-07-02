#include "private.hpp"
#include <block_memory_generated.hpp>
#include <tos/ae/kernel/platform_support.hpp>
#include <tos/ae/kernel/runners/preemptive_user_runner.hpp>
#include <tos/ae/registry.hpp>
#include <tos/periph/bcm2837_clock.hpp>

tos::ae::services::block_memory::sync_server* init_ephemeral_block();

tos::ae::registry_base& get_registry();

void platform_support::stage1_init() {
    tos::periph::clock_manager clock_man;
    LOG("CPU Freq:", clock_man.get_frequency(tos::bcm283x::clocks::arm));
    LOG("Max CPU Freq:", clock_man.get_max_frequency(tos::bcm283x::clocks::arm));
    clock_man.set_frequency(tos::bcm283x::clocks::arm,
                            clock_man.get_max_frequency(tos::bcm283x::clocks::arm));
    LOG("CPU Freq:", clock_man.get_frequency(tos::bcm283x::clocks::arm));
    m_chrono.emplace(ic);
}

void platform_support::stage2_init() {
    m_palloc = initialize_page_allocator();
    tos::launch(tos::alloc_stack,
                [this] { usb_task(ic, m_chrono.get().clock, m_chrono.get().alarm); });
    tos::ae::preemptive_user_group_runner::create(
        tos::make_erased_preemptive_runner(*this));

    get_registry().register_service("node_block", init_ephemeral_block());
}

// semihosting_output platform_support::init_serial() {
//     return {};
// }

tos::raspi3::uart0 platform_support::init_serial() {
    return {tos::uart::default_115200, ic};
}

platform_group_args platform_support::make_args() {
    return {.page_alloc = &*m_palloc,
            .trampoline = &*m_trampoline,
            .table = &tos::aarch64::get_current_translation_table()};
}
