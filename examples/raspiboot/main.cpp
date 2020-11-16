#include <arch/drivers.hpp>
#include <common/clock.hpp>
#include <tos/aarch64/assembly.hpp>
#include <tos/aarch64/exception.hpp>
#include <tos/aarch64/mmu.hpp>
#include <tos/debug/dynamic_log.hpp>
#include <tos/debug/log.hpp>
#include <tos/debug/sinks/serial_sink.hpp>
#include <tos/ft.hpp>
#include <tos/interrupt_trampoline.hpp>
#include <tos/periph/bcm2837_clock.hpp>
#include <tos/print.hpp>
#include <tos/soc/bcm2837.hpp>

void dump_tables() {
    auto& level0_table = *reinterpret_cast<tos::aarch64::translation_table*>(
        tos::aarch64::get_ttbr0_el1());

    LOG("Level0 table at", (void*)tos::aarch64::address_to_page(&level0_table));

    for (int i = 0; i < level0_table.entries.size(); ++i) {
        auto& entry = level0_table[i];
        if (entry.valid()) {
            LOG("Valid entry at", i);
            LOG("Num: ", (void*)entry.page_num());
            LOG("Leaf?", !entry.page());
        }
    }
}

[[gnu::noinline]] int fib(int x) {
    if (x <= 0) {
        return 1;
    }
    return x * fib(x - 1) * fib(x - 2);
}

void el0_fn() {
    asm volatile("mov x0, #0xDEAD");
    asm volatile("mov x30, #0xCAFE");
    tos::aarch64::svc1();
    while (true)
        ;
}

tos::stack_storage el0_stack;

void switch_to_el0() {
    LOG("Switching to user space...");
    LOG("Depth:", int(tos::global::disable_depth));

    uint64_t spsr_el1 = 0;
    tos::aarch64::set_spsr_el1(spsr_el1);
    tos::aarch64::set_elr_el1(reinterpret_cast<uintptr_t>(&el0_fn));
    tos::aarch64::set_sp_el0(reinterpret_cast<uintptr_t>(&el0_stack) + sizeof(el0_stack));
    tos::aarch64::isb();
    tos::aarch64::eret();
}

class svc_on_demand_interrupt {
public:
    template<class T>
    void operator()(T&& t) {
        tos::cur_arch::exception::set_svc_handler(
            tos::cur_arch::exception::svc_handler_t(t));
        tos::cur_arch::svc1();
    }
};

extern "C" {
int64_t _irq_count;
int64_t _irq_exit_count;
}

tos::kern::tcb* task;
tos::raspi3::uart0* uart_ptr;
tos::stack_storage thread_stack;
namespace debug = tos::debug;
void raspi_main() {
    task = tos::self();
    tos::raspi3::interrupt_controller ic;
    auto uart = tos::open(tos::devs::usart<0>, tos::uart::default_115200, ic);
    uart_ptr = &uart;
    uart.sync_write(tos::raw_cast(tos::span("Hello")));
    tos::debug::serial_sink uart_sink(&uart);
    tos::debug::detail::any_logger uart_log{&uart_sink};
    tos::debug::set_default_log(&uart_log);

    LOG("Log init complete");

    LOG("Hello from tos");

    auto serial = tos::raspi3::get_board_serial();
    LOG("Serial no:", serial);

    auto el = tos::aarch64::get_execution_level();
    LOG("ARM64 Execution Level:", el);

    tos::periph::clock_manager clock_man;
    LOG("CPU Freq:", clock_man.get_frequency(tos::bcm283x::clocks::arm));
    LOG("Max CPU Freq:", clock_man.get_max_frequency(tos::bcm283x::clocks::arm));
    clock_man.set_frequency(tos::bcm283x::clocks::arm,
                            clock_man.get_max_frequency(tos::bcm283x::clocks::arm));
    LOG("CPU Freq:", clock_man.get_frequency(tos::bcm283x::clocks::arm));

    tos::raspi3::system_timer timer(ic);
    //    tos::clock clock(&timer);
    tos::alarm alarm(&timer);

    dump_tables();

    auto& self = *tos::self();
    svc_on_demand_interrupt odi;
    auto trampoline = tos::make_interrupt_trampoline(odi);

    LOG("Trampoline setup complete");

    auto svc_handler_ = [&](int svnum, tos::aarch64::exception::stack_frame_t&) {
        //        LOG("Got svc", svnum);
        //        LOG("Switching back...");
        tos::kern::disable_interrupts();
        trampoline->switch_to(self);
    };

    auto old = tos::aarch64::exception::set_svc_handler(
        tos::aarch64::exception::svc_handler_t(svc_handler_));

    tos::launch(tos::alloc_stack, [&] { switch_to_el0(); });
    tos::kern::suspend_self(tos::int_guard{});
    LOG("User-kernel switch works!");
    tos::aarch64::exception::set_svc_handler(
        tos::aarch64::exception::svc_handler_t([](auto...) {}));
    //    tos::aarch64::udf();
    tos::aarch64::svc1();

    //    LOG("SP:", (void*)tos::aarch64::get_sp_el1());

    tos::launch(thread_stack, [&] {
        using namespace std::chrono_literals;
        while (true) {
            tos::this_thread::sleep_for(alarm, 10ms);
            LOG("Tick");
        }
    });
    using namespace std::chrono_literals;

    while (true) {
        uint8_t c = 'a';
        auto buf = uart->read(tos::monospan(c));
        uart->write(buf);
    }

    tos::this_thread::block_forever();
}

tos::stack_storage stack;
void tos_main() {
    tos::launch(stack, raspi_main);
}
