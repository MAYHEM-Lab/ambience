#include "../kernel.hpp"
#include "platform_support.hpp"
#include <alarm_generated.hpp>
#include <calc_generated.hpp>
#include <caplets_generated.hpp>
#include <timeout_generated.hpp>
#include <common/clock.hpp>
#include <common/timer.hpp>
#include <deque>
#include <file_system_generated.hpp>
#include <group1.hpp>
#include <lwip/init.h>
#include <lwip/timeouts.h>
#include <nonstd/variant.hpp>
#include <schema_generated.hpp>
#include <tos/ae/caplets_service_host.hpp>
#include <tos/ae/kernel/user_group.hpp>
#include <tos/ae/registry.hpp>
#include <tos/ae/transport/caplets/adapter.hpp>
#include <tos/ae/transport/downcall.hpp>
#include <tos/ae/transport/lwip/host.hpp>
#include <tos/ae/transport/lwip/udp.hpp>
#include <tos/async_init.hpp>
#include <tos/debug/log.hpp>
#include <tos/debug/sinks/lidl_sink.hpp>
#include <tos/detail/poll.hpp>
#include <tos/detail/tos_bind.hpp>
#include <tos/elf.hpp>
#include <tos/fixed_string.hpp>
#include <tos/gnu_build_id.hpp>
#include <tos/interrupt_trampoline.hpp>
#include <tos/lwip/common.hpp>
#include <tos/peripheral/uart_16550.hpp>
#include <tos/peripheral/vga_text.hpp>
#include <tos/task.hpp>
#include <tos/x86_64/apic.hpp>
#include <tos/x86_64/exception.hpp>
#include <tos/x86_64/mmu.hpp>
#include <tos/x86_64/pic.hpp>
#include <tos/x86_64/pit.hpp>
#include <tos/x86_64/syscall.hpp>

using tos::ae::service_mapping;
using tos::ae::service_registry;

using registry_t = service_registry<
    service_mapping<"calc", tos::ae::services::calculator::async_server*>,
    service_mapping<"logger", tos::services::logger::sync_server*>,
    service_mapping<"alarm", tos::ae::services::alarm::async_server*>,
    service_mapping<"fs", tos::ae::services::filesystem::sync_server*>>;

registry_t registry;

extern "C" {
void abort() {
    LOG_ERROR("Abort called");
    while (true)
        ;
}
}

tos::physical_page_allocator* g_palloc;

using errors = mpark::variant<page_alloc_res, nullptr_t>;

tos::expected<std::unique_ptr<tos::ae::kernel::user_group>, errors>
load_from_elf(const tos::elf::elf64& elf,
              tos::interrupt_trampoline& trampoline,
              tos::physical_page_allocator& palloc,
              tos::cur_arch::translation_table& root_table);

tos::expected<tos::physical_page_allocator*, page_alloc_res> initialize_page_allocator();

namespace {
struct scrollable : tos::self_pointing<scrollable> {
    void draw() {
        auto lines_to_draw = std::min<int>(m_vga.heigth, m_lines.size() - m_top_line);
        for (int i = m_top_line, j = 0; j < lines_to_draw; ++i, ++j) {
            m_vga.write_line(j, std::string_view(m_lines[i]));
        }
    }

    int write(tos::span<const uint8_t> data) {
        for (auto c : data) {
            if (c == '\n') {
                m_lines.emplace_back();
                m_lines.back().reserve(48);

                if (m_locked && m_lines.size() > m_vga.heigth) {
                    m_top_line += 1;
                }
                continue;
            } else if (c == '\r' || c == 0) {
                continue;
            }
            m_lines.back() += c;
        }
        draw();
        return data.size();
    }

    bool scroll_down() {
        if (m_top_line == m_lines.size() - 1) {
            return false;
        }
        m_top_line += 1;
        if (m_top_line == m_lines.size() - 1) {
            m_locked = true;
        }
        draw();
        return true;
    }

    bool scroll_up() {
        if (m_top_line == 0) {
            return false;
        }
        m_top_line -= 1;
        m_locked = false;
        draw();
        return true;
    }

    bool m_locked = true;
    int m_top_line = 0;
    std::vector<std::string> m_lines = std::vector<std::string>(1);
    tos::x86_64::text_vga m_vga{};
};

scrollable vga{};

void kb_isr(tos::x86_64::exception_frame* frame, int) {
    auto kc = tos::x86_64::port(0x60).inb();
    if (kc == 36) { // j
        vga.scroll_up();
    } else if (kc == 37) { // k
        vga.scroll_down();
    }
}

class x86_64_platform_support {
public:
    void stage1_init() {
    }

    void stage2_init() {
        auto apic_base = tos::x86_64::get_apic_base_address();
        LOG((void*)apic_base,
            (void*)tos::x86_64::rdmsr(tos::x86_64::msrs::ia32_apic_base));
        auto& table = tos::cur_arch::get_current_translation_table();
        auto seg = tos::segment{
            .range = {.base = apic_base, .size = tos::cur_arch::page_size_bytes},
            .perms = tos::permissions::read_write};
        Assert(tos::x86_64::map_region(table,
                                       seg,
                                       tos::user_accessible::no,
                                       tos::memory_types::device,
                                       m_palloc,
                                       reinterpret_cast<void*>(apic_base)));

        // IOAPIC registers
        seg = tos::segment{
            .range = {.base = 0xfec00000, .size = tos::cur_arch::page_size_bytes},
            .perms = tos::permissions::read_write};
        Assert(tos::x86_64::map_region(table,
                                       seg,
                                       tos::user_accessible::no,
                                       tos::memory_types::device,
                                       m_palloc,
                                       reinterpret_cast<void*>(0xfec00000)));

        auto& apic_regs = tos::x86_64::get_apic_registers(apic_base);
        LOG((void*)(uintptr_t)apic_regs.id, (void*)(uintptr_t)apic_regs.version);

        g_palloc = m_palloc;

        do_acpi_stuff();

        tos::x86_64::pic::disable();
        tos::x86_64::apic apic(apic_regs);
        apic.enable();
        LOG(apic_regs.tpr);
        apic_regs.tpr = 0;

        auto IOAPICID = tos::x86_64::ioapic_read((void*)0xfec00000, 0);
        LOG(IOAPICID);
        auto IOAPICVER = tos::x86_64::ioapic_read((void*)0xfec00000, 1);
        LOG((void*)IOAPICVER);

        // Timer
        tos::x86_64::ioapic_set_irq(2, apic_regs.id, 32 + 0);

        // Keyboard
        tos::x86_64::ioapic_set_irq(1, apic_regs.id, 32 + 1);

        ensure(tos::platform::take_irq(1));
        tos::platform::set_irq(1, tos::free_function_ref(+kb_isr));

        tos::lwip::global::system_clock = &m_clock;

        init_pci(*m_palloc, registry);

        set_name(tos::launch(tos::alloc_stack,
                             [&] {
                                 while (true) {
                                     using namespace std::chrono_literals;
                                     tos::this_thread::sleep_for(m_alarm, 50ms);
                                     tos::lock_guard lg{tos::lwip::lwip_lock};
                                     sys_check_timeouts();
                                 }
                             }),
                 "LWIP check timeouts");
    }

    auto init_serial() {
        //        return &vga;
        auto ser = tos::x86_64::uart_16550::open();
        if (!ser) {
            tos::println(vga, "Could not open uart");
            while (true)
                ;
        }
        return force_get(ser);
    }

    auto init_timer() {
        return m_tim_mux.channel(0);
    }

    auto init_odi() {
        return on_demand_interrupt();
    }

    auto init_physical_memory_allocator() {
        auto palloc = force_get(initialize_page_allocator());
        m_palloc = palloc;
        return palloc;
    }

    struct sample_group_descr {
        static constexpr auto& elf_body = group1;
        static constexpr auto services = tos::meta::list<tos::ae::services::calculator>{};
    };

    std::vector<std::unique_ptr<tos::ae::kernel::user_group>>
    init_groups(tos::interrupt_trampoline& trampoline,
                tos::physical_page_allocator& palloc) {
        auto& level0_table = tos::cur_arch::get_current_translation_table();

        std::vector<std::unique_ptr<tos::ae::kernel::user_group>> runnable_groups;
        {
            runnable_groups.emplace_back(tos::ae::kernel::load_preemptive_elf_group(
                sample_group_descr{}, trampoline, palloc, level0_table));

            LOG("Group loaded");
        }

        return runnable_groups;
    }

    template<class FnT>
    void set_syscall_handler(FnT&& syshandler) {
        tos::x86_64::set_syscall_handler(tos::x86_64::syscall_handler_t(syshandler));
    }

    void return_to_thread_from_irq(tos::kern::tcb& from, tos::kern::tcb& to) {
        return_from = &from;
        return_to = &to;
        tos::platform::set_post_irq(
            tos::mem_function_ref<&x86_64_platform_support::do_return>(*this));
    }

    void do_return(tos::x86_64::exception_frame*) {
        tos::platform::reset_post_irq();
        tos::swap_context(*return_from, *return_to, tos::int_ctx{});
    }

    tos::kern::tcb* return_from;
    tos::kern::tcb* return_to;

    tos::physical_page_allocator* m_palloc;

    using timer_mux_type = tos::timer_multiplexer<timer, 3>;
    using channel_type = timer_mux_type::multiplexed_timer;
    using clock_type = tos::clock<channel_type>;
    using erased_clock_type = tos::detail::erased_clock<clock_type>;

    using alarm_type = tos::alarm<channel_type>;
    using erased_alarm_type = tos::detail::erased_alarm<alarm_type>;

    timer_mux_type m_tim_mux;
    erased_clock_type m_clock{m_tim_mux.channel(1)};
    erased_alarm_type m_alarm{m_tim_mux.channel(2)};
};

using kernel_t = tos::ae::manager<x86_64_platform_support>;
} // namespace

struct priv_group {
    static void init(kernel_t& man) {
        static async_any_alarm_impl async_alarm{&man.m_alarm};
        registry.register_service<"alarm">(&async_alarm);

        static tos::debug::log_server serv(man.get_log_sink());
        registry.register_service<"logger">(&serv);
    }
};

struct sample_group {
    tos::ae::kernel::user_group* group;

    template<class Registry>
    tos::Task<void> init_dependencies(Registry& registry) {
        group->exposed_services.emplace_back(
            tos::ae::service_host(co_await registry.template wait<"logger">()));
        group->exposed_services.emplace_back(
            tos::ae::service_host(co_await registry.template wait<"alarm">()));
        group->exposed_services.emplace_back(
            tos::ae::service_host(co_await registry.template wait<"fs">()));

        // Wait for all dependencies to come online, then register our services

        registry.template register_service<"calc">(&calc());
    }

    tos::ae::services::calculator::async_server& calc() {
        return static_cast<tos::ae::services::calculator::async_server&>(
            *group->channels[0]);
    }
};

tos::expected<void, errors> kernel() {
    kernel_t man;
    man.initialize();
    priv_group::init(man);

    tos::coro::make_detached([]() -> tos::Task<void> {
        auto calc = co_await registry.wait<"calc">();
        auto res = co_await calc->add(3, 4);
        tos::launch(tos::alloc_stack, [res] { tos::debug::log("3 + 4 =", res); });
    }());

    auto sample_group = ::sample_group{man.groups().front().get()};

    tos::coro::make_detached(sample_group.init_dependencies(registry));

    //    tos::ae::lwip_host fs_server(tos::ae::service_host(registry.take<"fs">()),
    //                                 tos::port_num_t{1994});

    //    tos::ae::services::filesystem::stub_client<
    //        local_transport<tos::ae::sync_service_host>>
    //        sfs_client{tos::ae::service_host(fs)};

    //    tos::ae::sync_caplets_host<tos::ae::Token,
    //    tos::ae::services::filesystem::sync_server>
    //        cfs_server(fs);


    //
    //    tos::ae::services::filesystem::stub_client<
    //        caplets_adapter<tos::ae::Token, tos::ae::udp_transport>>
    //        fs_client{tos::udp_endpoint_t{tos::parse_ipv4_address("138.68.18.36"),
    //        {1994}}};
    //
    //    std::array<uint8_t, 128> mbbuf;
    //    lidl::message_builder mb(mbbuf);
    //    cfs_server.run_message(tos::span<uint8_t>{nullptr}, mb);

    tos::this_thread::yield();
    while (true) {
        man.run();
        tos::debug::log("Back");
    }

    return {};
}

static tos::stack_storage kern_stack;
void tos_main() {
    set_name(tos::launch(kern_stack, kernel), "Kernel");
}