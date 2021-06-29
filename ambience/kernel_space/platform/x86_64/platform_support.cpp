#include "private.hpp"
#include "scrollable.hpp"
#include <lwip/timeouts.h>
#include <tos/ae/kernel/platform_support.hpp>
#include <tos/ae/kernel/runners/preemptive_user_runner.hpp>
#include <tos/lwip/common.hpp>
#include <tos/lwip/lwip.hpp>
#include <tos/peripheral/uart_16550.hpp>
#include <tos/x86_64/port.hpp>

tos::physical_page_allocator* g_palloc;
tos::ae::registry_base& get_registry();

namespace {
scrollable vga{};

void kb_isr(tos::x86_64::exception_frame* frame, int) {
    auto kc = tos::x86_64::port(0x60).inb();
    if (kc == 36) { // j
        vga.scroll_up();
    } else if (kc == 37) { // k
        vga.scroll_down();
    }
}
} // namespace

void platform_support::stage2_init() {
    m_palloc = force_get(initialize_page_allocator());

    g_palloc = m_palloc;

    apic_initialize(*m_palloc);

    ensure(tos::platform::take_irq(1));
    tos::platform::set_irq(1, tos::free_function_ref(+kb_isr));

    tos::lwip::global::system_clock = &m_chrono.clock;

    init_pci(*m_palloc, get_registry());

    set_name(tos::launch(tos::alloc_stack,
                         [&] {
                             while (true) {
                                 using namespace std::chrono_literals;
                                 tos::this_thread::sleep_for(m_chrono.alarm, 50ms);
                                 tos::lock_guard lg{tos::lwip::lwip_lock};
                                 sys_check_timeouts();
                             }
                         }),
             "LWIP check timeouts");

    tos::ae::preemptive_user_group_runner::create(
        tos::make_erased_preemptive_runner(*this));
}

tos::x86_64::uart_16550 platform_support::init_serial() {
    //        return &vga;
    auto ser = tos::x86_64::uart_16550::open();
    if (!ser) {
        tos::println(vga, "Could not open uart");
        while (true)
            ;
    }
    return force_get(ser);
}

platform_group_args platform_support::make_args() {
    return {.page_alloc = &*m_palloc,
            .trampoline = &*m_trampoline,
            .table = &tos::x86_64::get_current_translation_table()};
}

namespace tos::ae::x86_64 {}