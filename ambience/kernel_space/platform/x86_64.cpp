#include <calc_generated.hpp>
#include <group1.hpp>
#include <nonstd/variant.hpp>
#include <tos/ae/kernel/user_group.hpp>
#include <tos/arch.hpp>
#include <tos/debug/dynamic_log.hpp>
#include <tos/debug/log.hpp>
#include <tos/debug/sinks/serial_sink.hpp>
#include <tos/elf.hpp>
#include <tos/interrupt_trampoline.hpp>
#include <tos/paging/physical_page_allocator.hpp>
#include <tos/peripheral/uart_16550.hpp>
#include <tos/peripheral/vga_text.hpp>
#include <tos/task.hpp>
#include <tos/x86_64/backtrace.hpp>
#include <tos/x86_64/exception.hpp>
#include <tos/x86_64/mmu.hpp>
#include <tos/x86_64/pic.hpp>
#include <tos/x86_64/pit.hpp>
#include <tos/x86_64/syscall.hpp>

extern "C" {
void abort() {
    LOG_ERROR("Abort called");
    while (true)
        ;
}
}

namespace {
using page_alloc_res = mpark::variant<tos::cur_arch::mmu_errors>;
using errors = mpark::variant<page_alloc_res, nullptr_t>;

tos::expected<tos::physical_page_allocator*, page_alloc_res> initialize_page_allocator() {
    auto vmem_end = (void*)tos::default_segments::image().end();

    LOG("Image ends at", vmem_end);

    auto allocator_space = tos::align_nearest_up_pow2(
        tos::physical_page_allocator::size_for_pages(1024), 4096);
    LOG("Physpage allocator would need", allocator_space, "bytes");

    auto allocator_segment =
        tos::segment{tos::memory_range{uintptr_t(vmem_end), ptrdiff_t(allocator_space)},
                     tos::permissions::read_write};

    EXPECTED_TRYV(
        tos::cur_arch::allocate_region(tos::cur_arch::get_current_translation_table(),
                                       allocator_segment,
                                       tos::user_accessible::no,
                                       nullptr));

    EXPECTED_TRYV(
        tos::cur_arch::mark_resident(tos::cur_arch::get_current_translation_table(),
                                     allocator_segment.range,
                                     tos::memory_types::normal,
                                     vmem_end));

    auto palloc = new (vmem_end) tos::physical_page_allocator(1024);

    palloc->mark_unavailable(tos::default_segments::image());
    palloc->mark_unavailable({0, 4096});
    palloc->mark_unavailable({0x00080000, 0x000FFFFF - 0x00080000});
    LOG("Available:", palloc, palloc->remaining_page_count());

    return palloc;
}

void switch_to_user(void* user_code) {
    using namespace tos::x86_64;
    asm volatile("add $8, %rsp");
    asm volatile("movq $0, %rbp");
    asm volatile("movq $0x202, %r11");
    sysret(user_code);
}

struct on_demand_interrupt {
    template<class T>
    void operator()(T&& t) {
        tos::platform::set_irq(12, tos::platform::irq_handler_t(t));
        tos::cur_arch::int0x2c();
    }
};

tos::expected<void, errors> map_elf(const tos::elf::elf64& elf,
                                    tos::physical_page_allocator& palloc,
                                    tos::cur_arch::translation_table& root_table) {
    for (auto pheader : elf.program_headers()) {
        if (pheader.type != tos::elf::segment_type::load) {
            continue;
        }

        LOG("Load",
            (void*)(uint32_t)pheader.file_size,
            "bytes from",
            (void*)(uint32_t)pheader.file_offset,
            "to",
            (void*)(uint32_t)pheader.virt_address);

        auto seg = elf.segment(pheader);

        void* base = const_cast<uint8_t*>(seg.data());
        if (pheader.file_size < pheader.virt_size) {
            auto pages = pheader.virt_size / tos::cur_arch::page_size_bytes;
            auto p = palloc.allocate(pages);
            base = palloc.address_of(*p);
        }

        LOG((void*)pheader.virt_address,
            pheader.virt_size,
            (void*)pheader.file_offset,
            pheader.file_size,
            base);

        auto vseg =
            tos::segment{.range = {.base = pheader.virt_address,
                                   .size = static_cast<ptrdiff_t>(pheader.virt_size)},
                         .perms = tos::permissions::all};

        EXPECTED_TRYV(tos::cur_arch::map_region(root_table,
                                                vseg,
                                                tos::user_accessible::yes,
                                                tos::memory_types::normal,
                                                &palloc,
                                                base));
    }

    return {};
}

tos::expected<tos::span<uint8_t>, errors>
create_and_map_stack(size_t stack_size,
                     tos::physical_page_allocator& palloc,
                     tos::cur_arch::translation_table& root_table) {
    stack_size = tos::align_nearest_up_pow2(stack_size, tos::cur_arch::page_size_bytes);
    auto page_count = stack_size / tos::cur_arch::page_size_bytes;
    auto stack_pages = palloc.allocate(page_count);

    if (!stack_pages) {
        return tos::unexpected(tos::cur_arch::mmu_errors::page_alloc_fail);
    }

    EXPECTED_TRYV(tos::cur_arch::map_region(
        root_table,
        tos::segment{.range = {.base = reinterpret_cast<uintptr_t>(
                                   palloc.address_of(*stack_pages)),
                               .size = static_cast<ptrdiff_t>(stack_size)},
                     tos::permissions::read_write},
        tos::user_accessible::yes,
        tos::memory_types::normal,
        &palloc,
        palloc.address_of(*stack_pages)));

    return tos::span<uint8_t>(static_cast<uint8_t*>(palloc.address_of(*stack_pages)),
                              stack_size);
}

tos::expected<tos::ae::kernel::user_group, errors> start_group(
    tos::span<uint8_t> stack, void (*entry)(), tos::interrupt_trampoline& trampoline) {
    LOG("Entry point:", (void*)entry);
    auto& user_thread = tos::suspended_launch(stack, switch_to_user, (void*)entry);

    auto& self = *tos::self();

    tos::ae::kernel::user_group res;
    res.state = &user_thread;
    auto syshandler = [&](tos::cur_arch::syscall_frame& frame) {
        assert(frame.rdi == 1);

        auto ifc = reinterpret_cast<tos::ae::interface*>(frame.rsi);
        res.iface.user_iface = ifc;

        trampoline.switch_to(self);
    };
    tos::x86_64::set_syscall_handler(tos::cur_arch::syscall_handler_t(syshandler));

    tos::swap_context(self, user_thread, tos::int_guard{});
    return res;
}

tos::expected<tos::ae::kernel::user_group, errors>
load_from_elf(const tos::elf::elf64& elf,
              tos::interrupt_trampoline& trampoline,
              tos::physical_page_allocator& palloc,
              tos::cur_arch::translation_table& root_table) {
    EXPECTED_TRYV(map_elf(elf, palloc, root_table));

    return start_group(EXPECTED_TRY(create_and_map_stack(
                           4 * tos::cur_arch::page_size_bytes, palloc, root_table)),
                       reinterpret_cast<void (*)()>(elf.header().entry),
                       trampoline);
}

tos::expected<void, errors> kernel() {
    tos::x86_64::text_vga vga;
    vga.clear();
    tos::println(vga, "Hello amd64 Tos!");

    auto uart = EXPECTED_TRY(tos::x86_64::uart_16550::open());
    tos::println(uart, "ambience");

    tos::debug::serial_sink uart_sink(&vga);
    tos::debug::detail::any_logger uart_log{&uart_sink};
    uart_log.set_log_level(tos::debug::log_level::trace);
    tos::debug::set_default_log(&uart_log);

    LOG("Log setup complete");

    tos::x86_64::pit timer;
    LOG("Timer initialized");

    auto palloc = EXPECTED_TRY(initialize_page_allocator());
    LOG("Page allocator initialized");

    on_demand_interrupt odi{};
    auto trampoline = tos::make_interrupt_trampoline(odi);
    LOG("Interrupt trampoline initialized");

    std::vector<tos::ae::kernel::user_group> runnable_groups;
    {
        LOG(group1.slice(0, 4));
        auto elf_res = tos::elf::elf64::from_buffer(group1);
        if (!elf_res) {
            LOG_ERROR("Could not parse payload!");
            LOG_ERROR("Error code: ", int(force_error(elf_res)));
            while (true)
                ;
        }

        runnable_groups.push_back(
            force_get(load_from_elf(force_get(elf_res),
                                    *trampoline,
                                    *palloc,
                                    tos::cur_arch::get_current_translation_table())));
    }

    LOG("Done loading");

    int32_t x = 3, y = 42;
    auto params = std::make_tuple(&x, &y);
    auto results = tos::ae::service::calculator::wire_types::add_results{-1};

    auto& req1 = tos::ae::submit_req<true>(
        *runnable_groups.front().iface.user_iface, 0, 0, &params, &results);

    auto& self = *tos::self();

    auto syshandler2 = [&](tos::x86_64::syscall_frame& frame) {
        tos::swap_context(*runnable_groups.front().state, self, tos::int_ctx{});
    };
    tos::x86_64::set_syscall_handler(tos::x86_64::syscall_handler_t(syshandler2));

    bool preempted = false;
    auto preempt = [&](tos::x86_64::exception_frame* frame) {
        tos::platform::reset_post_irq();
        preempted = true;
        tos::swap_context(*runnable_groups.front().state, self, tos::int_ctx{});
    };

    auto irq_handler = [&](tos::x86_64::exception_frame* frame, int) {
        tos::platform::set_post_irq(
            tos::function_ref<void(tos::x86_64::exception_frame*)>(preempt));
    };
    tos::platform::set_irq(
        0, tos::function_ref<void(tos::x86_64::exception_frame*, int)>(irq_handler));

    timer.set_frequency(100);

    for (int i = 0; i < 10; ++i) {
        LOG("back", results.ret0(), "Preempted", preempted);
        proc_req_queue(runnable_groups.front().iface);
        tos::this_thread::yield();

        odi([&](auto...) {
            preempted = false;

            timer.enable();
            tos::swap_context(self, *runnable_groups.front().state, tos::int_ctx{});
            timer.disable();
        });
        std::swap(runnable_groups.front(), runnable_groups.back());
    }

    return {};
}
} // namespace

void tos_main() {
    tos::launch(tos::alloc_stack, kernel);
}