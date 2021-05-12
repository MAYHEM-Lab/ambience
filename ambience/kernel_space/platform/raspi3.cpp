#include "kernel.hpp"
#include <arch/drivers.hpp>
#include <calc_generated.hpp>
#include <group1.hpp>
#include <nonstd/variant.hpp>
#include <tos/aarch64/exception.hpp>
#include <tos/aarch64/mmu.hpp>
#include <tos/ae/kernel/user_group.hpp>
#include <tos/ae/transport/downcall.hpp>
#include <tos/arch.hpp>
#include <tos/debug/dynamic_log.hpp>
#include <tos/debug/sinks/lidl_sink.hpp>
#include <tos/debug/sinks/serial_sink.hpp>
#include <tos/detail/tos_bind.hpp>
#include <tos/elf.hpp>
#include <tos/flags.hpp>
#include <tos/interrupt_trampoline.hpp>
#include <tos/late_constructed.hpp>
#include <tos/periph/bcm2837_clock.hpp>
#include <tos/platform.hpp>

using errors = mpark::variant<tos::aarch64::mmu_errors>;
using tos::expected;

expected<void, errors> map_elf(const tos::elf::elf64& elf,
                               tos::physical_page_allocator& palloc,
                               tos::cur_arch::translation_table& root_table) {
    for (auto pheader : elf.program_headers()) {
        if (pheader.type != tos::elf::segment_type::load) {
            continue;
        }

        LOG_TRACE("Load",
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

        tos::permissions perms = tos::permissions::read_write;
        if (tos::util::is_flag_set(pheader.attrs, tos::elf::segment_attrs::execute)) {
            perms = tos::permissions::read_execute;
        }

        auto vseg =
            tos::segment{.range = {.base = pheader.virt_address,
                                   .size = static_cast<ptrdiff_t>(pheader.virt_size)},
                         .perms = perms};

        EXPECTED_TRYV(tos::cur_arch::map_region(root_table,
                                                vseg,
                                                tos::user_accessible::yes,
                                                tos::memory_types::normal,
                                                &palloc,
                                                base));

        LOG_TRACE("OK");
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

class svc_on_demand_interrupt {
public:
    template<class T>
    void operator()(T&& t) {
        tos::cur_arch::exception::set_svc_handler(
            tos::cur_arch::exception::svc_handler_t(t));
        tos::cur_arch::svc1();
    }
};

void switch_to_el0(void (*el0_fn)(), void* stack, size_t stack_size) {
    LOG("Switching to EL0", (void*)el0_fn);
    tos::platform::disable_interrupts();
    uint64_t spsr_el1 = 0;
    tos::aarch64::set_spsr_el1(spsr_el1);
    tos::aarch64::set_elr_el1(reinterpret_cast<uintptr_t>(el0_fn));
    tos::aarch64::set_sp_el0(reinterpret_cast<uintptr_t>(stack) + stack_size);
    tos::aarch64::isb();
    tos::aarch64::eret();
}

alignas(16) uint8_t stk[1024];
expected<tos::ae::kernel::user_group, errors> start_group(
    tos::span<uint8_t> stack, void (*entry)(), tos::interrupt_trampoline& trampoline) {
    auto& self = *tos::self();

    tos::ae::kernel::user_group res;

    auto svc_handler_ = [&](int svnum, tos::cur_arch::exception::stack_frame_t& frame) {
        Assert(frame.gpr[0] == 1);

        auto ifc = reinterpret_cast<tos::ae::interface*>(frame.gpr[1]);
        res.iface.user_iface = ifc;

        trampoline.switch_to(self);
    };
    tos::aarch64::exception::set_svc_handler(
        tos::aarch64::exception::svc_handler_t(svc_handler_));

    res.state = &tos::suspended_launch(
        tos::alloc_stack, [&] { switch_to_el0(entry, stack.data(), stack.size()); });

    tos::swap_context(self, *res.state, tos::int_guard{});

    return res;
}

tos::expected<tos::ae::kernel::user_group, errors>
load_from_elf(const tos::elf::elf64& elf,
              tos::interrupt_trampoline& trampoline,
              tos::physical_page_allocator& palloc,
              tos::cur_arch::translation_table& root_table) {
    EXPECTED_TRYV(map_elf(elf, palloc, root_table));

    LOG_TRACE("ELF Mapped");

    auto stack = EXPECTED_TRY(
        create_and_map_stack(4 * tos::cur_arch::page_size_bytes, palloc, root_table));

    LOG_TRACE("Stack Mapped");

    return start_group(
        stack, reinterpret_cast<void (*)()>(elf.header().entry), trampoline);
}

class raspi3_platform_support {
public:
    void stage1_init() {
        tos::periph::clock_manager clock_man;
        LOG("CPU Freq:", clock_man.get_frequency(tos::bcm283x::clocks::arm));
        LOG("Max CPU Freq:", clock_man.get_max_frequency(tos::bcm283x::clocks::arm));
        clock_man.set_frequency(tos::bcm283x::clocks::arm,
                                clock_man.get_max_frequency(tos::bcm283x::clocks::arm));
        LOG("CPU Freq:", clock_man.get_frequency(tos::bcm283x::clocks::arm));
    }

    void stage2_init() {
    }

    auto init_serial() {
        return tos::open(tos::devs::usart<0>, tos::uart::default_115200, ic);
    }

    auto init_timer() {
        return tos::raspi3::generic_timer(ic, 0);
    }

    auto init_odi() {
        return svc_on_demand_interrupt();
    }

    auto init_physical_memory_allocator() {
        auto& level0_table = tos::cur_arch::get_current_translation_table();

        auto op_res = tos::aarch64::allocate_region(
            level0_table,
            tos::segment{{4096, 4096 * 5}, tos::permissions::read_write},
            tos::user_accessible::no,
            nullptr);
        if (!op_res) {
            LOG_ERROR("Could not allocate ...");
        }

        op_res = tos::aarch64::mark_resident(
            level0_table,
            tos::segment{{4096, 4096 * 5}, tos::permissions::read_write},
            tos::memory_types::normal,
            (void*)4096);
        if (!op_res) {
            LOG_ERROR("Could not mark resident ...");
        }

        tos::aarch64::tlb_invalidate_all();

        auto palloc =
            new (reinterpret_cast<void*>(4096)) tos::physical_page_allocator(1024);

        tos::aarch64::traverse_table_entries(
            level0_table, [&](tos::memory_range range, tos::aarch64::table_entry& entry) {
                LOG_TRACE("Making [",
                          (void*)range.base,
                          ",",
                          (void*)range.end(),
                          "] unavailable");

                entry.allow_user(true);
                palloc->mark_unavailable(range);
            });

        palloc->mark_unavailable({0, 4096});
        tos::aarch64::tlb_invalidate_all();

        return palloc;
    }

    std::vector<tos::ae::kernel::user_group>
    init_groups(tos::interrupt_trampoline& trampoline,
                tos::physical_page_allocator& palloc) {
        auto& level0_table = tos::cur_arch::get_current_translation_table();

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

            runnable_groups.push_back(force_get(
                load_from_elf(force_get(elf_res), trampoline, palloc, level0_table)));
            LOG("Group loaded");

            runnable_groups.back().channels.push_back(
                std::make_unique<tos::ae::services::calculator::async_zerocopy_client<
                    tos::ae::downcall_transport>>(
                    *runnable_groups.back().iface.user_iface, 0));
        }

        return runnable_groups;
    }

    template<class FnT>
    void set_syscall_handler(FnT&& syshandler) {
        tos::aarch64::exception::set_svc_handler(
            tos::aarch64::exception::svc_handler_t(syshandler));
    }

    void return_to_thread_from_irq(tos::kern::tcb& from, tos::kern::tcb& to) {
        return_from = &from;
        return_to = &to;
        ic.set_post_irq(
            tos::mem_function_ref<&raspi3_platform_support::do_return>(*this));
    }

private:
    void do_return() {
        ic.reset_post_irq();
        tos::swap_context(*return_from, *return_to, tos::int_ctx{});
    }

    tos::kern::tcb* return_from;
    tos::kern::tcb* return_to;

    tos::raspi3::interrupt_controller ic;
};

expected<void, errors> kernel() {
    tos::ae::manager<raspi3_platform_support> man;
    man.initialize();

    tos::debug::log_server serv(man.get_log_sink());

    man.groups().front().exposed_services.emplace_back(&serv);

    auto req_task = [&g = man.groups().front()]() -> tos::Task<void> {
        auto serv = static_cast<tos::ae::services::calculator::async_server*>(
            g.channels.front().get());

        auto res = co_await serv->add(3, 4);
        tos::launch(tos::alloc_stack, [res]{
            LOG("3 + 4 =", res);
        });
    };

    tos::coro_job j(tos::current_context(), tos::coro::make_pollable(req_task()));
    tos::kern::make_runnable(j);
    tos::this_thread::yield();

    for (int i = 0; i < 10; ++i) {
        man.run();
    }

    return {};
}

static tos::stack_storage kern_stack;
void tos_main() {
    tos::launch(kern_stack, kernel);
}