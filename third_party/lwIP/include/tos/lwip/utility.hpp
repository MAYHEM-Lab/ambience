#pragma once

#include <lwip/ip4_addr.h>
#include <common/inet/tcp_ip.hpp>

namespace tos::lwip {
/**
 * Converts a tos IPv4 address to an lwIP IPv4 address.
 * @param addr address to convert
 * @return lwIP address
 */
inline ::ip4_addr_t convert_address(const ipv4_addr_t& addr) {
    ip4_addr_t res;
    memcpy(&res.addr, addr.addr.data(), 4);
    return res;
}

/**
 * Converts an lwIP IPv4 address to a tos IPv4 address.
 * @param addr address to convert
 * @return tos address
 */
inline ipv4_addr_t convert_to_tos(const ip4_addr_t& addr) {
    ipv4_addr_t res;
    memcpy(res.addr.data(), &addr.addr, 4);
    return res;
}
} // namespace tos::lwip
