#include <arch/drivers.hpp>
#include <common/clock.hpp>
#include <common/inet/tcp_ip.hpp>
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

using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;

#include <lwip/etharp.h>
#include <lwip/init.h>
#include <lwip/tcp.h>
#include <numeric>
#include <tos/lwip/common.hpp>
#include <tos/lwip/tcp.hpp>
#include <tos/lwip/udp.hpp>
#include <tos/lwip/utility.hpp>
#include <uspi.h>
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

void el0_fn() {
    volatile int* p = nullptr;
    tos::aarch64::svc1();
    while (true) {
        tos::aarch64::svc1();
        *p = 42;
    }
}

alignas(4096) tos::stack_storage el0_stack;

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

namespace tos::lwip {
template<class DeviceT>
class basic_interface {
public:
    basic_interface(DeviceT&& dev,
                    const ipv4_addr_t& addr,
                    const ipv4_addr_t& mask,
                    const ipv4_addr_t& gw)
        : m_tap(std::move(dev)) {
        auto lwip_addr = convert_address(addr);
        auto lwip_mask = convert_address(mask);
        auto lwip_gw = convert_address(gw);
        netif_add(&m_if,
                  &lwip_addr,
                  &lwip_mask,
                  &lwip_gw,
                  this,
                  &basic_interface::init,
                  netif_input);
    }

    void up() {
        netif_set_link_up(&m_if);
        netif_set_up(&m_if);
    }

    void down() {
        netif_set_down(&m_if);
        netif_set_link_down(&m_if);
    }

    ~basic_interface() {
        down();
        netif_remove(&m_if);
    }

private:
    netif m_if;
    DeviceT m_tap;

    err_t init() {
        m_if.hostname = "tos";
        m_if.flags |= NETIF_FLAG_ETHARP | NETIF_FLAG_BROADCAST;
        m_if.linkoutput = &basic_interface::link_output;
        m_if.output = etharp_output;
        m_if.name[1] = m_if.name[0] = 'm';
        m_if.num = 0;
        m_if.mtu = 1500;
        m_if.hwaddr_len = 6;
        m_if.link_callback = &basic_interface::link_callback;
        m_if.status_callback = &basic_interface::status_callback;
        std::iota(std::begin(m_if.hwaddr), std::end(m_if.hwaddr), 1);
        launch(alloc_stack, [this] { read_thread(); });
        return ERR_OK;
    }

    void read_thread() {
        auto& tok = tos::cancellation_token::system();
        while (!tok.is_cancelled()) {
            std::vector<uint8_t> buf(1500);
            auto recvd = m_tap->read(buf);
            if (recvd.empty()) {
                continue;
            }
            LOG_TRACE("Received", recvd.size(), "bytes");
            auto p =
                pbuf_alloc(pbuf_layer::PBUF_LINK, recvd.size(), pbuf_type::PBUF_POOL);
            std::copy(recvd.begin(), recvd.end(), static_cast<uint8_t*>(p->payload));
            m_if.input(p, &m_if);
        }
    }

    err_t link_output(pbuf* p) {
        LOG_TRACE("link_output", p->len, "bytes");
        // pbuf_ref(p);
        // return m_if.input(p, &m_if);
        auto written = m_tap->write({static_cast<const uint8_t*>(p->payload), p->len});
        LOG_TRACE("Written", written, "bytes");
        return ERR_OK;
    }

    err_t output(pbuf* p, const ip4_addr_t* ipaddr) {
        LOG_TRACE("output", p->len, "bytes");
        // pbuf_ref(p);
        // return m_if.input(p, &m_if);
        return ERR_OK;
    }

    void link_callback() {
    }

    void status_callback() {
    }

private:
    static err_t init(struct netif* netif) {
        return static_cast<basic_interface*>(netif->state)->init();
    }

    static err_t link_output(struct netif* netif, struct pbuf* p) {
        return static_cast<basic_interface*>(netif->state)->link_output(p);
    }

    static err_t output(struct netif* netif, struct pbuf* p, const ip4_addr_t* ipaddr) {
        return static_cast<basic_interface*>(netif->state)->output(p, ipaddr);
    }

    static void link_callback(struct netif* netif) {
        static_cast<basic_interface*>(netif->state)->link_callback();
    }

    static void status_callback(struct netif* netif) {
        static_cast<basic_interface*>(netif->state)->status_callback();
    }

    friend void set_default(basic_interface& interface) {
        netif_set_default(&interface.m_if);
    }
};
} // namespace tos::lwip

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

    auto svc_handler_ = [&](int svnum, tos::aarch64::exception::stack_frame_t&) {
        tos::kern::disable_interrupts();
        trampoline->switch_to(self);
    };

    tos::aarch64::exception::set_svc_handler(
        tos::aarch64::exception::svc_handler_t(svc_handler_));

    auto table = std::make_unique<tos::aarch64::translation_table>();
    auto& tsk = tos::launch(tos::alloc_stack, [&] { switch_to_el0(); });
    tos::kern::suspend_self(tos::int_guard{});
    LOG("User-kernel switch works!");
    tos::aarch64::exception::set_svc_handler(
        tos::aarch64::exception::svc_handler_t([](auto...) {}));
    runnable.push_back(tsk);

    auto post_svc_handler = [&](int svnum,
                                tos::aarch64::exception::stack_frame_t& frame) {
        tos::swap_context(*tos::self(), self, tos::int_ctx{});
    };

    std::optional<tos::aarch64::exception::fault_variant> f;
    auto fault_handler = [&](const tos::aarch64::exception::fault_variant& fault,
                             tos::aarch64::exception::stack_frame_t& frame) {
        f = fault;
        tos::swap_context(*tos::self(), self, tos::int_ctx{});
    };

    while (true) {
        if (!runnable.empty()) {
            LOG("Will sched");
            auto& front = static_cast<tos::kern::tcb&>(runnable.front());
            runnable.pop_front();
            odi([&](auto&&...) {
                tos::aarch64::exception::set_fault_handler(
                    tos::aarch64::exception::fault_handler_t(fault_handler));
                tos::aarch64::exception::set_svc_handler(
                    tos::aarch64::exception::svc_handler_t(post_svc_handler));
                //                auto& old =
                //                tos::aarch64::set_current_translation_table(*table);
                tos::swap_context(*tos::self(), front, tos::int_ctx{});
                //                tos::aarch64::set_current_translation_table(old);
            });

            tos::aarch64::exception::set_svc_handler(
                tos::aarch64::exception::svc_handler_t([](auto...) {}));

            if (f) {
                using namespace tos::aarch64::exception;
                LOG_ERROR("User space had a fault!");
                std::visit(tos::make_overload(
                               [](const data_abort& data_fault) {
                                    LOG_ERROR("Data fault on", (void*)data_fault.data_addr);
                                    LOG_ERROR("With ISS", (void*)data_fault.iss);
                                    LOG_ERROR("At", (void*)data_fault.return_address);
                               },
                               [](const auto& unknown) {
                                 LOG_ERROR("Unknown fault");
                                 LOG_ERROR("At", (void*)unknown.return_address);}),
                           *f);
            } else {
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
