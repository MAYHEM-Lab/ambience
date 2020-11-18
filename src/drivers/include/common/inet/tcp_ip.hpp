//
// Created by fatih on 6/30/18.
//

#pragma once

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstring>
#include <tos/print.hpp>
#include <tos/span.hpp>

namespace tos {
/**
 * This class represents an IP port number
 */
struct port_num_t {
    uint16_t port;
};

struct alignas(std::uint32_t) ipv4_addr_t {
    std::array<uint8_t, 4> addr;
};

struct mac_addr_t {
    std::array<uint8_t, 6> addr;
};

inline bool operator==(const port_num_t& a, const port_num_t& b) {
    return a.port == b.port;
}

inline bool operator!=(const port_num_t& a, const port_num_t& b) {
    return a.port != b.port;
}

inline bool operator==(const ipv4_addr_t& a, const ipv4_addr_t& b) {
    return std::memcmp(a.addr.data(), b.addr.data(), 4) == 0;
}

inline bool operator!=(const ipv4_addr_t& a, const ipv4_addr_t& b) {
    return std::memcmp(a.addr.data(), b.addr.data(), 4) != 0;
}

/**
 * Given an IPv4 address string in the form of (XXX.YYY.ZZZ.TTT), returns an
 * ipv4_addr_t object corresponding to the given address.
 * @param addr the address string
 */
inline constexpr ipv4_addr_t parse_ipv4_address(tos::span<const char> addr);

struct udp_endpoint_t {
    ipv4_addr_t addr;
    port_num_t port;
};

template<class StreamT>
void print(StreamT& stream, const mac_addr_t& addr) {
    tos::print(stream,
               uintptr_t(addr.addr[0]),
               uintptr_t(addr.addr[1]),
               uintptr_t(addr.addr[2]),
               uintptr_t(addr.addr[3]),
               uintptr_t(addr.addr[4]),
               uintptr_t(addr.addr[5]),
               tos::separator(':'));
}

template<class StreamT>
void print(StreamT& stream, const ipv4_addr_t& addr) {
    tos::print(stream,
               int(addr.addr[0]),
               int(addr.addr[1]),
               int(addr.addr[2]),
               int(addr.addr[3]),
               tos::separator('.'));
}
} // namespace tos

namespace tos {
inline constexpr int atoi(tos::span<const char> chars) {
    int res = 0;
    for (auto p = chars.begin(); p != chars.end() && *p != 0; ++p) {
        res = res * 10 + *p - '0';
    }
    return res;
}

inline constexpr ipv4_addr_t parse_ipv4_address(tos::span<const char> addr) {
    ipv4_addr_t res{};

    auto it = addr.begin();
    auto end = it;
    while (end != addr.end() && *end != '.') ++end;
    res.addr[0] = atoi({it, end});

    it = ++end;
    end = it;
    while (end != addr.end() && *end != '.') ++end;
    res.addr[1] = atoi({it, end});

    it = ++end;
    end = it;
    while (end != addr.end() && *end != '.') ++end;
    res.addr[2] = atoi({it, end});

    it = ++end;
    end = it;
    while (end != addr.end() && *end != '.') ++end;
    res.addr[3] = atoi({it, end});

    return res;
}
} // namespace tos