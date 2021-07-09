#pragma once

#include <arch/udp.hpp>
#include <lidlrt/transport/common.hpp>

namespace tos::ae {
struct hosted_udp_transport : lidl::verbatim_transform {
    hosted_udp_transport(tos::udp_endpoint_t ep);

    std::vector<uint8_t> get_buffer();
    std::vector<uint8_t> send_receive(tos::span<uint8_t> buffer);

private:
    tos::hosted::udp_socket m_sock;
    tos::udp_endpoint_t m_ep;
};
}