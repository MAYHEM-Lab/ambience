#pragma once

#include <common/inet/tcp_ip.hpp>
#include <tos/lwip/lwip.hpp>
#include <tos/lwip/udp.hpp>
#include <vector>
#include <lidlrt/transport/common.hpp>

namespace tos::ae {
struct udp_transport : lidl::verbatim_transform {
public:
    udp_transport(tos::udp_endpoint_t ep);

    std::vector<uint8_t> get_buffer();

    void operator()(tos::lwip::events::recvfrom_t,
                    tos::lwip::async_udp_socket*,
                    const tos::udp_endpoint_t& from,
                    tos::lwip::buffer&& buf);

    tos::span<uint8_t> send_receive(tos::span<uint8_t> buffer);

private:
    tos::semaphore m_wait{0};
    tos::lwip::buffer m_buf;
    tos::udp_endpoint_t m_ep;
    tos::lwip::async_udp_socket m_sock;
};
} // namespace tos::ae