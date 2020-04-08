#pragma once

#include <common/inet/tcp_ip.hpp>
#include <cstdint>
#include <tos/span.hpp>

namespace tos {
struct datetime {
    uint32_t unix_seconds;
};

/*
       0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9  0  1
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |LI | VN  |Mode |    Stratum    |     Poll      |   Precision    |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                          Root  Delay                           |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                       Root  Dispersion                         |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                     Reference Identifier                       |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                                                                |
      |                    Reference Timestamp (64)                    |
      |                                                                |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                                                                |
      |                    Originate Timestamp (64)                    |
      |                                                                |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                                                                |
      |                     Receive Timestamp (64)                     |
      |                                                                |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                                                                |
      |                     Transmit Timestamp (64)                    |
      |                                                                |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                 Key Identifier (optional) (32)                 |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                                                                |
      |                                                                |
      |                 Message Digest (optional) (128)                |
      |                                                                |
      |                                                                |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */

namespace detail {
template <typename T>
T swap_endian(T u)
{
    union
    {
        T u;
        unsigned char u8[sizeof(T)];
    } source, dest;

    source.u = u;

    for (size_t k = 0; k < sizeof(T); k++)
        dest.u8[k] = source.u8[sizeof(T) - k - 1];

    return dest.u;
}
}

static constexpr auto seconds_from_1900_to_1970 = 2'208'988'800UL;
struct sntp_timestamp {
    uint32_t m_seconds;
    uint32_t m_fraction;

    [[nodiscard]]
    uint32_t seconds() const {
        return detail::swap_endian(m_seconds);
    }

    [[nodiscard]]
    uint32_t unix_seconds() const {
        return seconds() - seconds_from_1900_to_1970;
    }

    [[nodiscard]]
    uint32_t fraction() const {
        return detail::swap_endian(m_fraction);
    }
};
static_assert(sizeof(sntp_timestamp) == 8);

// RFC4330 Figure 1
struct [[gnu::packed]] sntp_packet_header {
    uint8_t li_vn_mode;
    uint8_t stratum;
    uint8_t poll;
    uint8_t precision;
    uint32_t root_delay;
    uint32_t root_dispersion;
    uint32_t reference_identifier;
    sntp_timestamp reference_timestamp;
    sntp_timestamp originate_timestamp;
    sntp_timestamp receive_timestamp;
    sntp_timestamp transmit_timestamp;
};

static_assert(offsetof(sntp_packet_header, receive_timestamp) == 32);

template<class UDPSocketT>
datetime get_sntp_time(UDPSocketT& sock, const udp_endpoint_t& endpoint) {
    sntp_packet_header header{};
    constexpr auto Version = 4U << 3U;
    constexpr auto Client = 3U;
    header.li_vn_mode = Version | Client;
    sock.send_to(tos::raw_cast(tos::monospan(header)), endpoint);

    udp_endpoint_t from;
    auto res = sock.receive_from(tos::raw_cast<uint8_t>(tos::monospan(header)), from);

    return {header.transmit_timestamp.unix_seconds()};
}
} // namespace tos
