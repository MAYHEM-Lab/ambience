#pragma once

#include <common/inet/tcp_ip.hpp>
#include <lwip/ip4_addr.h>
#include <lwip/pbuf.h>
#include <tos/expected.hpp>

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

enum class pbuf_errors
{

};

template<class It>
inline expected<void, pbuf_errors> copy_to_pbuf(It begin, It end, pbuf& head) {
    auto cur_link = &head;
    while (begin != end && cur_link) {
        begin =
            std::copy_n(begin, cur_link->len, static_cast<uint8_t*>(cur_link->payload));
        cur_link = cur_link->next;
    }
    return {};
}
} // namespace tos::lwip
