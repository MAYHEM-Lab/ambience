//
// Created by fatih on 12/26/19.
//

#pragma once

#include <common/inet/tcp_ip.hpp>
#include <nonstd/variant.hpp>
#include <ti/drivers/net/wifi/wlan.h>

namespace tos::cc32xx {
struct wifi_connected {
    SlWlanEventConnect_t ev;
    span<const char> ssid() const {
        return span<const char>{reinterpret_cast<const char*>(&ev.SsidName[0]),
                                size_t(ev.SsidLen)};
    }
};
struct wifi_disconnected {};
struct ip_acquired {
    ipv4_addr_t address;
    ipv4_addr_t gateway;
};

using events = mpark::variant<wifi_connected, wifi_disconnected, ip_acquired>;
} // namespace tos::cc32xx
