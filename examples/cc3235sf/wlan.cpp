//
// Created by fatih on 12/26/19.
//

#include <arch/wlan.hpp>
#include <common/inet/tcp_ip.hpp>
#include <ti/drivers/net/wifi/simplelink.h>
#include <tos/expected.hpp>

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
} // namespace tos::cc32xx