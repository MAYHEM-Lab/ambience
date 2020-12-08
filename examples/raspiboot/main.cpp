#include <arch/drivers.hpp>
#include <common/clock.hpp>
#include <common/inet/tcp_ip.hpp>
#include <lidl/service.hpp>
#include <lwip/init.h>
#include <map>
#include <tos/aarch64/exception.hpp>
#include <tos/aarch64/mmu.hpp>
#include <tos/aarch64/semihosting.hpp>
#include <tos/debug/dynamic_log.hpp>
#include <tos/debug/log.hpp>
#include <tos/debug/sinks/lidl_sink.hpp>
#include <tos/debug/sinks/serial_sink.hpp>
#include <tos/ft.hpp>
#include <tos/interrupt_trampoline.hpp>
#include <tos/lwip/if_adapter.hpp>
#include <tos/lwip/udp.hpp>
#include <tos/periph/bcm2837_clock.hpp>
#include <tos/print.hpp>
#include <tos/soc/bcm2837.hpp>
#include <uspi/devicenameservice.h>
#include <uspi/dwhcidevice.h>
#include <uspi/smsc951x.h>
#include <uspios.h>

void dump_tables() {
    auto& level0_table = tos::aarch64::get_current_translation_table();

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

int64_t
lidlcall(int64_t channel, uint8_t* buf, int64_t len, uint8_t* res_buf, int64_t res_len) {
    asm("mov x0, %[channel]\n"
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

std::array<uint8_t, 256> reqbuf;
std::array<uint8_t, 256> resbuf;
void el0_fn() {
    tos::aarch64::svc1();

    struct svc_transport {
        tos::span<uint8_t> get_buffer() {
            return reqbuf;
        }

        tos::span<uint8_t> send_receive(tos::span<uint8_t> buf) {
            auto len = lidlcall(1, buf.data(), buf.size(), resbuf.data(), resbuf.size());
            if (len <= 0) {
                return buf.slice(0, 0);
            }
            return tos::span(resbuf).slice(0, len);
        }
    };

    auto remote = tos::services::remote_logger<svc_transport>();
    tos::debug::lidl_sink snk(remote);
    tos::debug::detail::any_logger log(&snk);
    log.set_log_level(tos::debug::log_level::all);
    log.error("hello from user space");

    while (true) {
        log.info("Tick from user space");
    }
}

alignas(4096) tos::stack_storage el0_stack;

void switch_to_el0() {
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
    tos::debug::serial_sink uart_sink(&uart);
    tos::debug::detail::any_logger uart_log{&uart_sink};
    tos::debug::set_default_log(&uart_log);

    auto sink_service = std::make_unique<tos::debug::log_server>(uart_sink);
    auto run = lidl::make_procedure_runner<tos::services::logger>();

    std::array<uint8_t, 16> req{00,
                                0x00,
                                0x00,
                                0x00,
                                0x00,
                                0x00,
                                0x00,
                                0x00,
                                0x06,
                                0x00,
                                0x00,
                                0x00,
                                0x00,
                                0x00,
                                0x00,
                                0x00};

    std::array<uint8_t, 256> resp{};
    lidl::message_builder mb{resp};
    run(*sink_service, req, mb);
    sink_service->log_string("helloo");
    sink_service->finish();

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

    tos::launch(tos::alloc_stack, [&] {
        using namespace std::chrono_literals;
        while (true) {
            tos::this_thread::sleep_for(gentimalarm, 1s);
            LOG("Tick!");
        }
    });

    tos::raspi3::system_timer timer(ic);

    tos::alarm alarm(&timer);
    auto erased = tos::erase_alarm(&alarm);
    global::alarm = erased.get();

    tos::launch(tos::alloc_stack, usb_task);

    using namespace std::chrono_literals;

    dump_tables();

    auto& self = *tos::self();
    svc_on_demand_interrupt odi;
    auto trampoline = tos::make_interrupt_trampoline(odi);

    LOG("Trampoline setup complete");

    tos::launch(thread_stack, [&] {
        using namespace std::chrono_literals;
        while (true) {
            tos::this_thread::sleep_for(alarm, 10ms);
            LOG("Tick");
        }
    });

    tos::launch(tos::alloc_stack, [&] {
        while (true) {
            uint8_t c = 'a';
            auto buf = uart->read(tos::monospan(c));
            uart->write(buf);
        }
    });

    tos::intrusive_list<tos::job> runnable;

    auto svc_handler_ = [&](int svnum, tos::cur_arch::exception::stack_frame_t&) {
        tos::kern::disable_interrupts();
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

    struct user_context : tos::static_context<> {
        tos::aarch64::exception::svc_handler_t m_svc_handler;
        std::optional<tos::aarch64::exception::fault_variant> m_fault;
        tos::kern::tcb* m_scheduler;

        user_context(tos::aarch64::exception::svc_handler_t svc_handler,
                     tos::kern::tcb& scheduler)
            : m_svc_handler(svc_handler)
            , m_scheduler(&scheduler) {
        }

        void switch_in() override {
            m_old_fault_handler = tos::aarch64::exception::set_fault_handler(
                tos::mem_function_ref<&user_context::fault>(*this));
            m_old_svc_handler = tos::aarch64::exception::set_svc_handler(m_svc_handler);
        }

        void switch_out() override {
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

    auto& tsk = tos::launch(tos::alloc_stack, [&] { switch_to_el0(); });
    tsk.set_context(user_ctx);

    tos::kern::suspend_self(tos::int_guard{});
    LOG("User-kernel switch works!");
    tos::aarch64::exception::set_svc_handler(
        tos::aarch64::exception::svc_handler_t([](auto...) {}));
    runnable.push_back(tsk);

    user_ctx.m_svc_handler = tos::aarch64::exception::svc_handler_t(post_svc_handler);

    while (true) {
        if (!runnable.empty()) {
//            LOG("Will sched");
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
