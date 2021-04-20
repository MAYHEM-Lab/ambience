#include <arch/drivers.hpp>
#include <calc_generated.hpp>
#include <group1.hpp>
#include <nonstd/variant.hpp>
#include <tos/aarch64/exception.hpp>
#include <tos/aarch64/mmu.hpp>
#include <tos/ae/kernel/user_group.hpp>
#include <tos/arch.hpp>
#include <tos/debug/dynamic_log.hpp>
#include <tos/debug/sinks/serial_sink.hpp>
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
    auto stage1_init() {
        tos::periph::clock_manager clock_man;
        LOG("CPU Freq:", clock_man.get_frequency(tos::bcm283x::clocks::arm));
        LOG("Max CPU Freq:", clock_man.get_max_frequency(tos::bcm283x::clocks::arm));
        clock_man.set_frequency(tos::bcm283x::clocks::arm,
                                clock_man.get_max_frequency(tos::bcm283x::clocks::arm));
        LOG("CPU Freq:", clock_man.get_frequency(tos::bcm283x::clocks::arm));
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
        }

        return runnable_groups;
    }

private:
    tos::raspi3::interrupt_controller ic;
};

template<class T>
concept PlatformSupport = requires(T t) {
    t.stage1_init();
    t.init_serial();
    t.init_timer();
    t.init_odi();
    t.init_physical_memory_allocator();
    t.init_groups(std::declval<tos::interrupt_trampoline&>(),
                  *t.init_physical_memory_allocator());
};

template<PlatformSupport Platform>
class manager : Platform {
    using serial_type = decltype(std::declval<Platform>().init_serial());
    using timer_type = decltype(std::declval<Platform>().init_timer());
    using odi_type = decltype(std::declval<Platform>().init_odi());

public:
    manager() {
    }

    void initialize() {
        m_serial.emplace_fn(&Platform::init_serial, static_cast<Platform*>(this));
        tos::println(m_serial.get(), "ambience");

        m_sink.emplace(&m_serial.get());
        m_logger.emplace(&m_sink.get());
        tos::debug::set_default_log(&m_logger.get());
        LOG("Log setup complete");

        // Stage 1 initialization has a logger!
        Platform::stage1_init();

        m_timer.emplace_fn(&Platform::init_timer, static_cast<Platform*>(this));
        LOG("Timer setup complete");

        m_odi.emplace_fn(&Platform::init_odi, static_cast<Platform*>(this));
        m_trampoline = tos::make_interrupt_trampoline(m_odi.get());
        LOG("Trampoline setup complete");

        auto palloc = Platform::init_physical_memory_allocator();
        LOG("Page allocator intialized");

        m_runnable_groups = Platform::init_groups(*m_trampoline, *palloc);
        LOG("Groups initialized");

        m_self = tos::self();
    }

    void run() {
        int32_t x = 3, y = 42;
        auto params = std::make_tuple(&x, &y);
        auto results = tos::ae::service::calculator::wire_types::add_results{-1};

        auto& req1 = tos::ae::submit_req<true>(
            *m_runnable_groups.front().iface.user_iface, 0, 0, &params, &results);

        auto syshandler2 = [this](auto&&...) {
            tos::swap_context(*m_runnable_groups.front().state, *m_self, tos::int_ctx{});
        };

        bool preempted = false;
        auto preempt = [&] {
            preempted = true;
            tos::swap_context(*m_runnable_groups.front().state, *m_self, tos::int_ctx{});
        };
        m_timer.get().set_callback(tos::function_ref<void()>(preempt));
        m_timer.get().set_frequency(100);

        for (int i = 0; i < 10; ++i) {
            LOG("back", results.ret0(), "Preempted", preempted);
            proc_req_queue(m_runnable_groups.front().iface);
            tos::this_thread::yield();

            m_odi.get()([&](auto...) {
                tos::aarch64::exception::set_svc_handler(
                    tos::aarch64::exception::svc_handler_t(syshandler2));

                preempted = false;
                m_timer.get().enable();
                tos::swap_context(
                    *m_self, *m_runnable_groups.front().state, tos::int_ctx{});
                m_timer.get().disable();
            });
            // std::swap(runnable_groups.front(), runnable_groups.back());
        }
    }

    ~manager() {
    }

private:
    tos::kern::tcb* m_self;
    std::unique_ptr<tos::interrupt_trampoline> m_trampoline;
    std::vector<tos::ae::kernel::user_group> m_runnable_groups;

    tos::late_constructed<serial_type> m_serial;
    tos::late_constructed<tos::debug::serial_sink<serial_type*>> m_sink;
    tos::late_constructed<tos::debug::detail::any_logger> m_logger;
    tos::late_constructed<timer_type> m_timer;
    tos::late_constructed<odi_type> m_odi;
};

expected<void, errors> kernel() {
    manager<raspi3_platform_support> man;
    man.initialize();
    man.run();

    return {};
}

static tos::stack_storage kern_stack;
void tos_main() {
    tos::launch(kern_stack, kernel);
}