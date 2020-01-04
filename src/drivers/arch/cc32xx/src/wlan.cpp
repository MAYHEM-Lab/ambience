//
// Created by fatih on 12/26/19.
//

#include "shared_variables.hpp"

#include <arch/detail/sock_rt.hpp>
#include <arch/wlan.hpp>
#include <common/inet/tcp_ip.hpp>
#include <common/usart.hpp>
#include <ti/drivers/SPI.h>
#include <ti/drivers/net/wifi/simplelink.h>
#include <tos/expected.hpp>
#include <tos/semaphore.hpp>

namespace tos::cc32xx {
tos::mac_addr_t get_mac_address() {
    tos::mac_addr_t mac;
    uint16_t macAddressLen;
    macAddressLen = sizeof(mac.addr);
    sl_NetCfgGet(SL_NETCFG_MAC_ADDRESS_GET, nullptr, &macAddressLen, &mac.addr[0]);
    return mac;
}

tos::expected<void, network_errors> set_mac_address(tos::mac_addr_t address) {
    auto res = sl_NetCfgSet(
        SL_NETCFG_MAC_ADDRESS_SET, 0, address.addr.size(), address.addr.data());
    if (res >= 0) {
        return {};
    }
    return tos::unexpected(network_errors(res));
}

simplelink_wifi::simplelink_wifi(tos::any_usart& log) {
    launch(stack_size_t{4096}, [this, &log] {
        SPI_init();
        thread(log);
    });
}

simplelink_wifi::~simplelink_wifi() {
}

void simplelink_wifi::thread(tos::any_usart& log) {
    using namespace tos::cc32xx;
    auto start_res = sl_Start(nullptr, nullptr, nullptr);
    tos::println(log, start_res);
    auto set_mode = sl_WlanSetMode(ROLE_STA);
    tos::println(log, set_mode);
    auto stop = sl_Stop(0);
    tos::println(log, stop);
    start_res = sl_Start(nullptr, nullptr, nullptr);
    tos::println(log, start_res);

    SlDeviceVersion_t firmwareVersion{};

    uint8_t ucConfigOpt = SL_DEVICE_GENERAL_VERSION;
    uint16_t ucConfigLen = sizeof(firmwareVersion);
    auto retVal = sl_DeviceGet(
        SL_DEVICE_GENERAL, &ucConfigOpt, &ucConfigLen, (uint8_t*)(&firmwareVersion));

    tos::println(log, "Host Driver Version:", SL_DRIVER_VERSION);
    tos::println(log,
                 "Build Version",
                 int(firmwareVersion.NwpVersion[0]),
                 int(firmwareVersion.NwpVersion[1]),
                 int(firmwareVersion.NwpVersion[2]),
                 int(firmwareVersion.NwpVersion[3]),
                 int(firmwareVersion.FwVersion[0]),
                 int(firmwareVersion.FwVersion[1]),
                 int(firmwareVersion.FwVersion[2]),
                 int(firmwareVersion.FwVersion[3]),
                 int(firmwareVersion.PhyVersion[0]),
                 int(firmwareVersion.PhyVersion[1]),
                 int(firmwareVersion.PhyVersion[2]),
                 int(firmwareVersion.PhyVersion[3]),
                 tos::separator('.'));

    auto set_res = set_mac_address({0xDA, 0x53, 0x83, 0x81, 0x41, 0x6B});
    if (!set_res) {
        tos::println(log, "Can't set mac address!");
    }

    auto mac = get_mac_address();
    tos::println(log, "Mac Address:", mac);

    const char* password = "serdar1988";
    SlWlanSecParams_t SecParams;
    SecParams.Type = SL_WLAN_SEC_TYPE_WPA_WPA2;
    SecParams.Key = reinterpret_cast<int8_t*>(const_cast<char*>(password));
    SecParams.KeyLen = strlen(password);

    const char* name = "Nakedsense.2";
    auto res = sl_WlanConnect(reinterpret_cast<const int8_t*>(name),
                              std::strlen(name),
                              nullptr,
                              &SecParams,
                              nullptr);

    tos::println(log, "connect:", int(res));

    using namespace tos::cc32xx;

    tos::launch(tos::alloc_stack, [] {
        while (true) {
            socket_runtime::instance().run();
        }
    });

    while (true) {
        sl_Task(nullptr);

        while (!evq.empty()) {
            auto ev = evq.pop();
            mpark::visit(tos::make_overload(
                             [&](const wifi_connected& ev) {
                                 tos::println(log, "Connected:", ev.ssid());
                                 m_ev_handler->handle(ev);
                             },
                             [&](const ip_acquired& ev) {
                                 tos::println(log, "Acquired IP:", ev.address);
                                 tos::println(log, "Gateway IP:", ev.gateway);
                                 m_ev_handler->handle(ev);
                             },
                             [&](const auto& ev) {
                                 tos::println(log, "Unhandled event");
                                 m_ev_handler->handle(ev);
                             }),
                         ev);
        }

        loop_sem.down();
    }
}
} // namespace tos::cc32xx