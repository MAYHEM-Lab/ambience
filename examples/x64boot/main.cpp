#include <cstddef>
#include <deque>
#include <tos/address_space.hpp>
#include <tos/debug/dynamic_log.hpp>
#include <tos/debug/sinks/serial_sink.hpp>
#include <tos/detail/tos_bind.hpp>
#include <tos/flags.hpp>
#include <tos/ft.hpp>
#include <tos/interrupt_trampoline.hpp>
#include <tos/mem_stream.hpp>
#include <tos/peripheral/uart_16550.hpp>
#include <tos/peripheral/vga_text.hpp>
#include <tos/physical_memory_backing.hpp>
#include <tos/suspended_launch.hpp>
#include <tos/x86_64/assembly.hpp>
#include <tos/x86_64/cpuid.hpp>
#include <tos/x86_64/mmu.hpp>
#include <tos/x86_64/pic.hpp>
#include <tos/x86_64/syscall.hpp>

void dump_table(tos::cur_arch::translation_table& table) {
    tos::cur_arch::traverse_table_entries(
        table, [](tos::memory_range range, tos::cur_arch::table_entry& entry) {
            LOG_TRACE((void*)entry.raw());
            LOG_TRACE(
                "VirtAddress:", "[", (void*)(range.base), ",", (void*)(range.end()), ")");
            LOG_TRACE("PhysAddress:",
                      (void*)tos::cur_arch::page_to_address(entry.page_num()));
            char perm_string[4] = "R__";
            auto perms = tos::cur_arch::translate_permissions(entry);
            if (tos::util::is_flag_set(perms, tos::permissions::write)) {
                perm_string[1] = 'W';
            }
            if (tos::util::is_flag_set(perms, tos::permissions::execute)) {
                perm_string[2] = 'X';
            }
            LOG_TRACE("Perms:", perm_string, "User:", entry.allow_user());
        });
}

void switch_to_user(void* user_code) {
    using namespace tos::x86_64;
    asm volatile("movq $0x202, %r11");
    sysret(user_code);
}

struct on_demand_interrupt {
    int irq;
    on_demand_interrupt() {
        irq = 12;
        ensure(tos::platform::take_irq(irq));
    }
    template<class T>
    void operator()(T&& t) {
        tos::platform::set_irq(irq, tos::platform::irq_handler_t(t));
        tos::cur_arch::int0x2c();
    }
};

void thread() {
    auto& self = *tos::self();

    auto uart_res = tos::x86_64::uart_16550::open();
    if (!uart_res) {
        tos::debug::panic("Could not open the uart");
    }
    auto& uart = force_get(uart_res);
    tos::println(uart, "Booted");

    tos::x86_64::text_vga vga;
    vga.clear();
    tos::println(vga, "Hello amd64 Tos!");

    tos::debug::serial_sink uart_sink(&uart);
    tos::debug::detail::any_logger uart_log{&uart_sink};
    uart_log.set_log_level(tos::debug::log_level::trace);
    tos::debug::set_default_log(&uart_log);

    tos::x86_64::pic::enable_irq(0);

    LOG("Hello world!");

    LOG(tos::x86_64::cpuid::manufacturer().data());
    on_demand_interrupt odi{};
    auto trampoline = tos::make_interrupt_trampoline(odi);

    auto cr3 = tos::x86_64::read_cr3();
    LOG("Page table at:", (void*)cr3);

    tos::physical_memory_backing pmem(
        tos::segment{tos::memory_range{.base = 0, .size = 1'000'000'000},
                     tos::permissions::all},
        tos::memory_types::normal);

    auto& level0_table = tos::cur_arch::get_current_translation_table();

    dump_table(level0_table);

    auto vmem_end = (void*)tos::default_segments::image().end();

    LOG("Image ends at", vmem_end);

    auto allocator_space =
        tos::align_nearest_up_pow2(tos::physical_page_allocator::size_for_pages(1024),
                                   tos::cur_arch::page_size_bytes);
    LOG("Physpage allocator would need", allocator_space, "bytes");

    auto allocator_segment =
        tos::segment{tos::memory_range{uintptr_t(vmem_end), ptrdiff_t(allocator_space)},
                     tos::permissions::read_write};

    auto op_res =
        tos::cur_arch::map_region(tos::cur_arch::get_current_translation_table(),
                                  allocator_segment,
                                  tos::user_accessible::no,
                                  tos::memory_types::normal,
                                  nullptr,
                                  vmem_end);
    LOG(bool(op_res));

    auto palloc = new (vmem_end) tos::physical_page_allocator(1024);
    palloc->mark_unavailable(tos::default_segments::image());
    palloc->mark_unavailable({0, tos::cur_arch::page_size_bytes});
    palloc->mark_unavailable({0x00080000, 0x000FFFFF - 0x00080000});
    LOG("Available:", palloc, palloc->remaining_page_count());

    auto vas = tos::cur_arch::address_space::adopt(level0_table);
    tos::global::cur_as = &vas;

    tos::mapping mapping;
    Assert(pmem.create_mapping(
        tos::segment{
            tos::memory_range{.base = 0x1200000, .size = tos::cur_arch::page_size_bytes},
            tos::permissions::read_write},
        tos::memory_range{.base = 0x200000, .size = tos::cur_arch::page_size_bytes},
        mapping));

    vas.do_mapping(mapping, palloc);

    LOG(*((int*)0x200000));
    LOG(*((int*)0x1200000));

    tos::semaphore sem{0};

    auto tim_handler = [&](tos::x86_64::exception_frame* frame, int) {
        LOG("Tick", frame->cs, frame->ss);
        sem.up_isr();
    };

    tos::platform::set_irq(
        0, tos::function_ref<void(tos::x86_64::exception_frame*, int)>(tim_handler));

    while (true) {
        sem.down();
        LOG("Tick");
    }
}

tos::stack_storage store;
void tos_main() {
    tos::launch(store, thread);
}