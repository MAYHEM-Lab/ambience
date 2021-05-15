#include <arch/interrupts.hpp>
#include <common/clock.hpp>
#include <lwip/init.h>
#include <lwip/timeouts.h>
#include <memory>
#include <tos/debug/log.hpp>
#include <tos/expected.hpp>
#include <tos/ft.inl>
#include <tos/lwip/if_adapter.hpp>
#include <tos/lwip/udp.hpp>
#include <tos/self_pointing.hpp>
#include <tos/tcb.hpp>
#include <uspi.h>
#include <uspi/devicenameservice.h>
#include <uspi/dwhcidevice.h>
#include <uspi/smsc951x.h>

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


namespace global {
tos::raspi3::interrupt_controller* ic;
tos::any_clock* clk;
tos::any_alarm* alarm;
} // namespace global


tos::expected<void, usb_errors> usb_task(tos::raspi3::interrupt_controller& ic,
                                         tos::any_clock& clk,
                                         tos::any_alarm& alarm) {
    global::ic = &ic;
    global::clk = &clk;
    global::alarm = &alarm;

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

    LOG("Initialize lwip");
    lwip_init();

    set_name(tos::launch(tos::alloc_stack,
                         [&] {
                             while (true) {
                                 using namespace std::chrono_literals;
                                 auto alarm_ptr = &alarm;
                                 tos::this_thread::sleep_for(alarm_ptr, 50ms);
                                 tos::lock_guard lg{tos::lwip::lwip_lock};
                                 sys_check_timeouts();
                             }
                         }),
             "LWIP check timeouts");

    tos::lwip::basic_interface interface(eth.get());
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

    auto res = sock.bind({9090});
    Assert(res);

    tos::this_thread::block_forever();

    return {};
}
