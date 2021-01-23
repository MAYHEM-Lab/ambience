#include <alarm_generated.hpp>
#include <arch/drivers.hpp>
#include <common/clock.hpp>
#include <common/inet/tcp_ip.hpp>
#include <lidlrt/service.hpp>
#include <lwip/init.h>
#include <map>
#include <tos/aarch64/exception.hpp>
#include <tos/aarch64/mmu.hpp>
#include <tos/aarch64/semihosting.hpp>
#include <tos/debug/dynamic_log.hpp>
#include <tos/debug/log.hpp>
#include <tos/debug/sinks/lidl_sink.hpp>
#include <tos/debug/sinks/serial_sink.hpp>
#include <tos/flags.hpp>
#include <tos/ft.hpp>
#include <tos/interrupt_trampoline.hpp>
#include <tos/lwip/if_adapter.hpp>
#include <tos/lwip/udp.hpp>
#include <tos/mem_stream.hpp>
#include <tos/periph/bcm2837_clock.hpp>
#include <tos/print.hpp>
#include <tos/soc/bcm2837.hpp>
#include <uspi/devicenameservice.h>
#include <uspi/dwhcidevice.h>
#include <uspi/smsc951x.h>
#include <uspios.h>

void dump_table(tos::aarch64::translation_table& table) {
    tos::aarch64::traverse_table_entries(
        table, [](tos::memory_range range, tos::aarch64::table_entry& entry) {
            LOG("VirtAddress:", "[", (void*)(range.base), ",", (void*)(range.end()), ")");
            LOG("PhysAddress:", (void*)tos::aarch64::page_to_address(entry.page_num()));
            char perm_string[4] = "R__";
            auto perms = tos::aarch64::translate_permissions(entry);
            if (tos::util::is_flag_set(perms, tos::permissions::write)) {
                perm_string[1] = 'W';
            }
            if (tos::util::is_flag_set(perms, tos::permissions::execute)) {
                perm_string[2] = 'X';
            }
            LOG("Perms:", perm_string, "User:", entry.allow_user());
        });
}

[[gnu::noinline]] int fib(int x) {
    if (x <= 0) {
        return 1;
    }
    return x * fib(x - 1) * fib(x - 2);
}

int64_t
lidlcall(int64_t channel, uint8_t* buf, int64_t len, uint8_t* res_buf, int64_t res_len) {
    asm volatile("mov x0, %[channel]\n"
                 "mov x1, %[buf]\n"
                 "mov x2, %[len]\n"
                 "mov x3, %[res_buf]\n"
                 "mov x4, %[res_len]\n"
                 :
                 : [channel] "r"(channel),
                   [buf] "r"(buf),
                   [len] "r"(len),
                   [res_buf] "r"(res_buf),
                   [res_len] "r"(res_len)
                 : "x0", "x1", "x2", "x3", "x4");
    tos::aarch64::svc1();
    int64_t result;
    asm volatile("mov %[result], x0" : [result] "=r"(result) : : "x0");
    return result;
}

template<int ChannelId>
struct svc_transport {
    std::array<uint8_t, 256> reqbuf;
    std::array<uint8_t, 256> resbuf;

    tos::span<uint8_t> get_buffer() {
        return reqbuf;
    }

    tos::span<uint8_t> send_receive(tos::span<uint8_t> buf) {
        auto len =
            lidlcall(ChannelId, buf.data(), buf.size(), resbuf.data(), resbuf.size());
        if (len <= 0) {
            return buf.slice(0, 0);
        }
        return tos::span(resbuf).slice(0, len);
    }
};

void el0_fn() {
    tos::aarch64::svc1();

    auto remote = tos::services::remote_logger<svc_transport<1>>();
    tos::debug::lidl_sink snk(remote);
    tos::debug::detail::any_logger log(&snk);
    log.set_log_level(tos::debug::log_level::all);
    log.error("hello from user space");

    auto current = tos::services::remote_current<svc_transport<3>>();
    log.info(current.get_thread_handle().id());

    /*auto our_addr_space = current.get_address_space();

    auto vm = tos::services::remote_virtual_memory<svc_transport<2>>();

    std::array<uint8_t, 128> buf;
    lidl::message_builder builder(buf);
    auto& regs = vm.get_regions(our_addr_space, builder);

    for (auto& reg : regs) {
        log.info(
            "Region [", reg.begin(), ",", reg.end(), ") with perms ", reg.permissions());
    }*/

    int i = 0;
    while (true) {
        log.info("Tick from user space", ++i);
    }
}

alignas(4096) tos::stack_storage el0_stack;

void switch_to_el0() {
    tos::platform::disable_interrupts();
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

enum class usb_errors
{
    hci_init_fail,
    no_device
};

class usb_hci {
public:
    static tos::expected<std::unique_ptr<usb_hci>, usb_errors> open() {
        auto ptr = std::make_unique<usb_hci>();

        if (!DWHCIDeviceInitialize(&ptr->DWHCI)) {
            LOG_WARN("Cannot initialize USB host controller interface");

            return tos::unexpected(usb_errors::hci_init_fail);
        }

        return ptr;
    }

    usb_hci() {
        DeviceNameService(&NameService);
        DWHCIDevice(&DWHCI);
    }

    ~usb_hci() {
        _DWHCIDevice(&DWHCI);
        _DeviceNameService(&NameService);
    }

    TDeviceNameService& get_name_service() {
        return NameService;
    }

private:
    TDeviceNameService NameService;
    TDWHCIDevice DWHCI;
};

class smsc951x : public tos::self_pointing<smsc951x> {
public:
    static tos::expected<std::unique_ptr<smsc951x>, usb_errors> open(usb_hci& hci) {
        auto ptr = DeviceNameServiceGetDevice(&hci.get_name_service(), "eth0", FALSE);
        if (!ptr) {
            return tos::unexpected(usb_errors::no_device);
        }

        auto res = std::make_unique<smsc951x>();
        res->m_ptr = static_cast<TSMSC951xDevice*>(ptr);
        return res;
    }

    bool is_up() const {
        return SMSC951xDeviceIsLinkUp(m_ptr);
    }

    tos::span<uint8_t> read(tos::span<uint8_t> buf) {
        Assert(buf.size() >= 1500);
        unsigned len;
        if (!SMSC951xDeviceReceiveFrame(m_ptr, buf.data(), &len)) {
            return tos::span<uint8_t>(nullptr);
        }
        return buf.slice(0, len);
    }

    int write(tos::span<const uint8_t> buf) {
        auto res = SMSC951xDeviceSendFrame(m_ptr, buf.data(), buf.size());
        if (!res) {
            return 0;
        }
        return buf.size();
    }

private:
    TSMSC951xDevice* m_ptr;
};

static const char FromSample[] = "sample";
tos::kern::tcb* usb_task_ptr;
tos::expected<void, usb_errors> usb_task() {
    usb_task_ptr = tos::self();

    auto hci = EXPECTED_TRY(usb_hci::open());

    LOG("Init done!");

    auto eth = EXPECTED_TRY(smsc951x::open(*hci));

    LOG("Eth Init done!");

    unsigned nTimeout = 0;
    while (!eth->is_up()) {
        MsDelay(100);

        if (++nTimeout < 40) {
            continue;
        }
        nTimeout = 0;

        LOG("Link is down");
    }

    LOG("Link is up!");

    auto ip = tos::parse_ipv4_address("192.168.0.250");
    tos::mac_addr_t addr{{0xDE, 0xAD, 0xBE, 0xEF, 0x66, 0xFF}};
    LOG("Initialize lwip");
    lwip_init();
    tos::lwip::basic_interface interface(eth.get(),
                                         tos::parse_ipv4_address("192.168.0.250"),
                                         tos::parse_ipv4_address("255.255.255.0"),
                                         tos::parse_ipv4_address("192.168.0.249"));
    set_default(interface);
    interface.up();

    tos::lwip::async_udp_socket sock;

    auto handler = [](auto, auto, auto, tos::lwip::buffer buf) {
        LOG("Received", buf.size(), "bytes!");
        std::vector<uint8_t> data(buf.size());
        buf.read(data);
        std::string_view sv(reinterpret_cast<char*>(data.data()), data.size());
        LOG(sv);
    };

    sock.attach(handler);

    auto res = sock.bind({9090}, tos::parse_ipv4_address("192.168.0.250"));
    Assert(res);

    tos::this_thread::block_forever();

    return {};
}

namespace global {
tos::raspi3::interrupt_controller* ic;
tos::any_clock* clk;
tos::any_alarm* alarm;
} // namespace global

namespace tos::aarch64 {
/*
 * This class implements the timer functionality of the arm generic timer.
 *
 * Since this is at the core level, how its interrupts are handled depends on the
 * platform, and therefore, this class is meant to be used in the driver implementation
 * of a platform rather than being directly used.
 *
 * There exists 1 of these timers per core, and they are not memory mapped. This means
 * that these timers cannot be shared across cores.
 */
class generic_timer {
public:
    static uint64_t get_counter() {
        return get_cntpct_el0();
    }

    static uint64_t get_frequency() {
        return get_cntfrq_el0();
    }

    static void set_timeout(uint32_t ticks) {
        set_cntp_tval_el0(ticks);
    }

    static void enable() {
        set_cntp_ctl_el0(1);
    }

    static void disable() {
        set_cntp_ctl_el0(0);
    }
};
} // namespace tos::aarch64

namespace tos::raspi3 {
class generic_timer : public self_pointing<generic_timer> {
public:
    generic_timer(interrupt_controller& ic, int core_num)
        : m_handler(mem_function_ref<&generic_timer::irq>(*this))
        , m_core{core_num} {
        ic.register_handler(bcm283x::irq_channels::generic_timer, m_handler);
    }

    void enable();
    void disable();

    void set_frequency(uint16_t hz);
    void set_callback(function_ref<void()> fn) {
        m_fn = fn;
    }

private:
    bool irq();

    irq_handler m_handler;
    int m_core;
    uint64_t m_period;
    function_ref<void()> m_fn{[](auto...) {}};
};
} // namespace tos::raspi3

namespace tos::raspi3 {
void generic_timer::set_frequency(uint16_t hz) {
    m_period = aarch64::generic_timer::get_frequency() / hz;
}

void generic_timer::enable() {
    // Enable the SVC generic timer interrupt.
    bcm2837::ARM_CORE->core0_timers_irq_control = 2;
    bcm2837::INTERRUPT_CONTROLLER->enable_basic_irq = 1;
    aarch64::generic_timer::set_timeout(m_period);
    aarch64::generic_timer::enable();
}

void generic_timer::disable() {
    aarch64::generic_timer::disable();
    bcm2837::INTERRUPT_CONTROLLER->disable_basic_irq = 1;
}

bool generic_timer::irq() {
    m_fn();
    aarch64::generic_timer::set_timeout(m_period);
    return true;
}
} // namespace tos::raspi3

enum class service_errors
{
    not_found,
    not_supported,
};

class any_service_host {
public:
    // Essentially a list of channels
    // Channels are self identifying
    // Dynamic service registration depends on the actual implementation

    virtual auto register_service(lidl::erased_procedure_runner_t runner,
                                  std::unique_ptr<lidl::service_base> serv)
        -> tos::expected<int, service_errors> = 0;

    virtual auto unregister_service(int channel_id)
        -> tos::expected<void, service_errors> = 0;

    virtual auto get_service(int channel_id)
        -> tos::expected<std::pair<lidl::service_base*, lidl::erased_procedure_runner_t>,
                         service_errors> = 0;

    virtual ~any_service_host() = default;
};

class dynamic_service_host : public any_service_host {
public:
    auto register_service(lidl::erased_procedure_runner_t runner,
                          std::unique_ptr<lidl::service_base> serv)
        -> tos::expected<int, service_errors> override {
        auto id = m_next_id++;
        m_services.emplace(id, std::make_pair(std::move(serv), runner));
        return id;
    }

    auto unregister_service(int channel_id)
        -> tos::expected<void, service_errors> override {
        return tos::unexpected(service_errors::not_supported);
    }

    auto get_service(int channel_id)
        -> tos::expected<std::pair<lidl::service_base*, lidl::erased_procedure_runner_t>,
                         service_errors> override {
        auto it = m_services.find(channel_id);
        if (it == m_services.end()) {
            return tos::unexpected(service_errors::not_found);
        }
        return std::make_pair(it->second.first.get(), it->second.second);
    }

private:
    int m_next_id = 1;
    std::map<
        int,
        std::pair<std::unique_ptr<lidl::service_base>, lidl::erased_procedure_runner_t>>
        m_services;
};

class semihosting_output : public tos::self_pointing<semihosting_output> {
public:
    int write(tos::span<const uint8_t> data) {
        auto res = data.size();
        while (!data.empty()) {
            auto len = std::min(data.size(), m_buf.size() - 1);
            auto it = std::copy_n(data.begin(), len, m_buf.begin());
            *it = 0;
            tos::aarch64::semihosting::write0(m_buf.data());
            data = data.slice(len);
        }
        return res;
    }

private:
    std::array<char, 128> m_buf;
};

tos::kern::tcb* task;
tos::raspi3::uart0* uart_ptr;
tos::stack_storage thread_stack;
namespace debug = tos::debug;
void raspi_main() {
    task = tos::self();
    tos::raspi3::interrupt_controller ic;
    global::ic = &ic;

    auto uart = tos::open(tos::devs::usart<0>, tos::uart::default_115200, ic);
    uart_ptr = &uart;
    uart.sync_write(tos::raw_cast(tos::span("Hello")));
    semihosting_output out;
    tos::debug::serial_sink uart_sink(&out);
    tos::debug::detail::any_logger uart_log{&uart_sink};
    tos::debug::set_default_log(&uart_log);

    auto sink_service = std::make_unique<tos::debug::log_server>(uart_sink);

    dynamic_service_host serv_host;
    serv_host.register_service(
        lidl::make_erased_procedure_runner<tos::services::logger>(),
        std::move(sink_service));

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

    tos::raspi3::generic_timer gentim(ic, 0);
    LOG("Generic timer initialized");
    tos::alarm gentimalarm(&gentim);
    LOG("Alarm initialized");

    tos::raspi3::system_timer timer(ic);

    tos::alarm alarm(&timer);
    auto erased = tos::erase_alarm(&alarm);
    global::alarm = erased.get();

    tos::launch(tos::alloc_stack, [&] {
        using namespace std::chrono_literals;
        while (true) {
            tos::this_thread::sleep_for(gentimalarm, 10ms);
            LOG("Tick!");
        }
    });

    tos::launch(thread_stack, [&] {
        using namespace std::chrono_literals;
        while (true) {
            tos::this_thread::sleep_for(alarm, 10ms);
            LOG("Tick");
        }
    });

    tos::launch(tos::alloc_stack, usb_task);

    using namespace std::chrono_literals;

    auto& level0_table = tos::aarch64::get_current_translation_table();

    dump_table(level0_table);

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

    LOG("Ready");

    auto palloc = new (reinterpret_cast<void*>(4096)) tos::physical_page_allocator(1024);


    tos::aarch64::traverse_table_entries(
        level0_table, [&](tos::memory_range range, tos::aarch64::table_entry& entry) {
          LOG("Making [",
              (void*)range.base,
              ",",
              (void*)range.end(),
              "] unavailable");

          palloc->mark_unavailable(range);
        });
    {
        tos::raspi3::property_channel_tags_builder builder;
        auto buf = builder.add(0x10005, {0, 0}).end();
        tos::raspi3::property_channel property;
        if (!property.transaction(buf)) {
            tos::debug::panic("Can't get ARM Memory");
        }

        LOG("ARM Base:", (void*)buf[0]);
        LOG("Len:", (void*)buf[1]);
    }

    {
        tos::raspi3::property_channel_tags_builder builder;
        auto buf = builder.add(0x10006, {0, 0}).end();
        tos::raspi3::property_channel property;
        if (!property.transaction(buf)) {
            tos::debug::panic("Can't get VC Memory");
        }

        LOG("VC Base:", (void*)buf[0]);
        LOG("Len:", (void*)buf[1]);
    }

    palloc->mark_unavailable({0, 4096});
    auto p = palloc->allocate(1);

    LOG("Allocated:", p.get());
    LOG("Address of page:", palloc->address_of(*p));

    op_res = tos::aarch64::allocate_region(
        level0_table,
        tos::segment{{(uintptr_t)palloc->address_of(*p), 4096},
                     tos::permissions::read_write},
        tos::user_accessible::no,
        nullptr);
    if (!op_res) {
        LOG_ERROR("Could not allocate ...");
    }

    op_res = tos::aarch64::mark_resident(
        level0_table,
        tos::segment{{(uintptr_t)palloc->address_of(*p), 4096},
                     tos::permissions::read_write},
        tos::memory_types::normal,
        palloc->address_of(*p));
    if (!op_res) {
        LOG_ERROR("Could not mark resident ...");
    }

    tos::aarch64::tlb_invalidate_all();

    (*(int*)palloc->address_of(*p)) = 42;

    dump_table(level0_table);

    auto& self = *tos::self();
    svc_on_demand_interrupt odi;
    auto trampoline = tos::make_interrupt_trampoline(odi);

    LOG("Trampoline setup complete");

    tos::launch(tos::alloc_stack, [&] {
        while (true) {
            uint8_t c = 'a';
            auto buf = uart->read(tos::monospan(c));
            uart->write(buf);
        }
    });

    tos::intrusive_list<tos::job> runnable;

    auto svc_handler_ = [&](int svnum, tos::cur_arch::exception::stack_frame_t&) {
        trampoline->switch_to(self);
    };

    int num;
    tos::cur_arch::exception::stack_frame_t* svframe = nullptr;
    auto post_svc_handler = [&](int svnum,
                                tos::cur_arch::exception::stack_frame_t& frame) {
        num = svnum;
        svframe = &frame;
        tos::swap_context(*tos::self(), self, tos::int_ctx{});
    };

    struct current_impl : tos::services::current {
        tos::services::handle<tos::services::address_space> get_address_space() override {
            return {-1};
        }

        tos::services::handle<tos::services::thread> get_thread_handle() override {
            return {42};
        }
    };

    serv_host.register_service(
        lidl::make_erased_procedure_runner<tos::services::current>(),
        std::make_unique<current_impl>());

    serv_host.register_service(
        lidl::make_erased_procedure_runner<tos::services::current>(),
        std::make_unique<current_impl>());

    struct user_context : tos::static_context<> {
        tos::aarch64::exception::svc_handler_t m_svc_handler;
        std::optional<tos::aarch64::exception::fault_variant> m_fault;
        tos::kern::tcb* m_scheduler;
        tos::aarch64::translation_table* m_trans_table;
        tos::aarch64::translation_table* m_old;

        user_context(tos::aarch64::exception::svc_handler_t svc_handler,
                     tos::kern::tcb& scheduler)
            : m_svc_handler(svc_handler)
            , m_scheduler(&scheduler) {
        }

        void switch_in() override {
            m_old_fault_handler = tos::aarch64::exception::set_fault_handler(
                tos::mem_function_ref<&user_context::fault>(*this));
            m_old_svc_handler = tos::aarch64::exception::set_svc_handler(m_svc_handler);
            m_old = &tos::aarch64::set_current_translation_table(*m_trans_table);
        }

        void switch_out() override {
            tos::aarch64::set_current_translation_table(*m_old);
            tos::aarch64::exception::set_fault_handler(m_old_fault_handler);
            tos::aarch64::exception::set_svc_handler(m_old_svc_handler);
        }

        void fault(const tos::aarch64::exception::fault_variant& fault,
                   tos::aarch64::exception::stack_frame_t& frame) {
            m_fault = fault;
            tos::swap_context(*tos::self(), *m_scheduler, tos::int_ctx{});
        }

    private:
        tos::aarch64::exception::svc_handler_t m_old_svc_handler{[](auto...) {}};
        tos::aarch64::exception::fault_handler_t m_old_fault_handler{[](auto...) {}};
    };

    user_context user_ctx{tos::aarch64::exception::svc_handler_t(svc_handler_), self};

    LOG("Allocating page table for user space");
    auto mem = tos::aarch64::recursive_table_clone(level0_table, *palloc);
    if (!mem) {
        LOG_ERROR("Could not create page table for user space");
        return;
    }
    user_ctx.m_trans_table = force_get(mem);
    tos::aarch64::traverse_table_entries(
        *user_ctx.m_trans_table,
        [](const tos::memory_range& range, tos::aarch64::table_entry& entry) {
            auto perms = tos::aarch64::translate_permissions(entry);
            if (tos::util::is_flag_set(perms, tos::permissions::write)) {
                LOG("Making [",
                    (void*)range.base,
                    ",",
                    (void*)range.end(),
                    "] inaccessible");
                entry.allow_user(false);
            }
            if (tos::contains(range, reinterpret_cast<uintptr_t>(&el0_stack)) ||
                tos::contains(
                    range, reinterpret_cast<uintptr_t>(&el0_stack) + sizeof(el0_stack) - 1) ||
                tos::contains(
                    range, reinterpret_cast<uintptr_t>(&el0_stack) + sizeof(el0_stack))) {
                LOG("Allowing access to stack [",
                    (void*)range.base,
                    ",",
                    (void*)range.end(),
                    "]");
                entry.allow_user(true);
            }
        });
    dump_table(*user_ctx.m_trans_table);

    auto& tsk = tos::launch(tos::alloc_stack, [&] { switch_to_el0(); });
    tsk.set_context(user_ctx);

    tos::kern::suspend_self(tos::int_guard{});

    LOG("User-kernel switch works!");
    tsk.get_context().switch_out();
    self.get_context().switch_in();

    tos::aarch64::exception::set_svc_handler(
        tos::aarch64::exception::svc_handler_t([](auto...) {}));
    runnable.push_back(tsk);

    user_ctx.m_svc_handler = tos::aarch64::exception::svc_handler_t(post_svc_handler);

    while (true) {
        if (!runnable.empty()) {
            auto& front = static_cast<tos::kern::tcb&>(runnable.front());
            runnable.pop_front();
            odi([&](auto&&...) {
                self.get_context().switch_out();
                front.get_context().switch_in();
                tos::swap_context(*tos::self(), front, tos::int_ctx{});
                front.get_context().switch_out();
                self.get_context().switch_in();
            });

            if (user_ctx.m_fault) {
                using namespace tos::aarch64::exception;
                LOG_ERROR("User space had a fault!");
                std::visit(tos::make_overload(
                               [](const data_abort& data_fault) {
                                   LOG_ERROR("Data fault on",
                                             (void*)data_fault.data_addr);
                                   LOG_ERROR("With ISS", (void*)data_fault.iss);
                                   if (data_fault.data_addr <= 64) {
                                       LOG_ERROR("(Null Pointer Access)");
                                   }
                                   LOG_ERROR("At", (void*)data_fault.return_address);
                               },
                               [](const auto& unknown) {
                                   LOG_ERROR("Unknown fault");
                                   LOG_ERROR("At", (void*)unknown.return_address);
                               }),
                           *user_ctx.m_fault);
            } else if (svframe) {
                for (auto& reg : svframe->gpr) {
                    //                    LOG((void*)reg);
                }

                int channel = svframe->gpr[0];
                auto ptr = reinterpret_cast<uint8_t*>(svframe->gpr[1]);
                auto len = svframe->gpr[2];
                auto res_ptr = reinterpret_cast<uint8_t*>(svframe->gpr[3]);
                auto res_len = svframe->gpr[2];
                tos::span<uint8_t> reqmem{ptr, len};
                tos::span<uint8_t> resmem{res_ptr, res_len};

                //                LOG("Got call on channel", channel);

                auto s = serv_host.get_service(channel);
                if (!s) {
                    LOG_WARN("Channel does not exist", channel);
                    svframe->gpr[0] = -1;
                } else {
                    auto [serv, runner] = force_get(s);

                    //                    LOG("Buffer:", reqmem);
                    lidl::message_builder mb(resmem);
                    runner(*serv, reqmem, mb);
                    svframe->gpr[0] = mb.size();
                }

                runnable.push_back(front);
                svframe = nullptr;
            } else {
                LOG_WARN("No svc frame!");
                runnable.push_back(front);
            }
        }
        tos::this_thread::yield();
    }

    tos::this_thread::block_forever();
}

tos::stack_storage stack;
void tos_main() {
    tos::launch(stack, raspi_main);
}
