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
#include <tos/debug/log.hpp>
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

static tos::stack_storage<4096> wifi_stack;
simplelink_wifi::simplelink_wifi() {
    launch(wifi_stack, [this] {
        SPI_init();
        thread();
    });
    m_working.down();
}

simplelink_wifi::~simplelink_wifi() {
    sl_Stop(1);
}

static tos::stack_storage<4096> srt_stack;
void simplelink_wifi::thread() {
    using namespace tos::cc32xx;
    LOG_TRACE("Calling sl_Start");
    auto start_res = sl_Start(nullptr, nullptr, nullptr);
    LOG_TRACE(start_res);
    auto set_mode = sl_WlanSetMode(ROLE_STA);
    LOG_TRACE(set_mode);
    auto stop = sl_Stop(1);
    LOG_TRACE(stop);
    start_res = sl_Start(nullptr, nullptr, nullptr);
    LOG_TRACE(start_res);

    SlDeviceVersion_t firmwareVersion{};

    uint8_t ucConfigOpt = SL_DEVICE_GENERAL_VERSION;
    uint16_t ucConfigLen = sizeof(firmwareVersion);
    auto retVal = sl_DeviceGet(
        SL_DEVICE_GENERAL, &ucConfigOpt, &ucConfigLen, (uint8_t*)(&firmwareVersion));

    LOG_TRACE("Host driver version:", SL_DRIVER_VERSION);

    LOG_TRACE("Build Version",
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
              int(firmwareVersion.PhyVersion[3]));

    auto set_res = set_mac_address({0xDA, 0x53, 0x83, 0x81, 0x41, 0x6B});
    if (!set_res) {
        LOG_ERROR("Can't set mac address!");
    }

    auto mac = get_mac_address();
    // tos::debug::info("Mac Address:", mac);

    using namespace tos::cc32xx;

    tos::launch(srt_stack, [] {
        while (true) {
            socket_runtime::instance().run();
        }
    });

    m_working.up();

    while (true) {
        sl_Task(nullptr);

        while (!evq.empty()) {
            auto ev = evq.pop();
            mpark::visit(tos::make_overload(
                             [&](const wifi_connected& ev) {
                                 LOG("Connected:", ev.ssid());
                                 m_ev_handler->handle(ev);
                             },
                             [&](const ip_acquired& ev) {
                                 // tos::debug::log("Acquired IP:", ev.address);
                                 // tos::debug::log("Gateway IP:", ev.gateway);
                                 m_ev_handler->handle(ev);
                             },
                             [&](const auto& ev) {
                                 LOG_WARN("Unhandled event");
                                 m_ev_handler->handle(ev);
                             }),
                         ev);
        }

        loop_sem.down();
    }
}

void simplelink_wifi::connect(std::string_view SSID, std::string_view password) {
    SlWlanSecParams_t SecParams;
    SecParams.Type = SL_WLAN_SEC_TYPE_WPA_WPA2;
    SecParams.Key = reinterpret_cast<int8_t*>(const_cast<char*>(password.data()));
    SecParams.KeyLen = password.size();

    auto res = sl_WlanConnect(reinterpret_cast<const int8_t*>(SSID.data()),
                              SSID.size(),
                              nullptr,
                              &SecParams,
                              nullptr);

    LOG("Connect:", int(res));
}

void simplelink_wifi::set_power_policy(power_policy policy) {
    int policy_res;
    switch (policy) {
    case power_policy::always_on:
        sl_WlanPolicySet(SL_WLAN_POLICY_PM, SL_WLAN_ALWAYS_ON_POLICY, nullptr, 0);
        break;
    case power_policy::low_latency:
        sl_WlanPolicySet(SL_WLAN_POLICY_PM, SL_WLAN_LOW_LATENCY_POLICY, nullptr, 0);
        break;
    case power_policy::normal:
        policy_res =
            sl_WlanPolicySet(SL_WLAN_POLICY_PM, SL_WLAN_NORMAL_POLICY, nullptr, 0);
        break;
    case power_policy::power_save:
        policy_res =
            sl_WlanPolicySet(SL_WLAN_POLICY_PM, SL_WLAN_LOW_POWER_POLICY, nullptr, 0);
        break;
    case power_policy::long_sleep_interval: {
        SlWlanPmPolicyParams_t PmPolicyParams;
        memset(&PmPolicyParams, 0, sizeof(SlWlanPmPolicyParams_t));
        PmPolicyParams.MaxSleepTimeMs = 60'000; // max sleep time in mSec
        policy_res = sl_WlanPolicySet(SL_WLAN_POLICY_PM,
                                      SL_WLAN_LONG_SLEEP_INTERVAL_POLICY,
                                      (_u8*)&PmPolicyParams,
                                      sizeof(PmPolicyParams));

    } break;
    }
    LOG_TRACE("policy:", policy_res);
}

void null_event_handler::handle(const ip_acquired& acquired) {
}
void null_event_handler::handle(const wifi_connected& connected) {
}
void null_event_handler::handle(const wifi_disconnected& disconnected) {
}
} // namespace tos::cc32xx
