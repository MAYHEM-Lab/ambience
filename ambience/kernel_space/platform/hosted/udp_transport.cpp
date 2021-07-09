#include <tos/ae/transport/hosted/udp.hpp>
#include <tos/platform.hpp>

namespace tos::ae {
hosted_udp_transport::hosted_udp_transport(tos::udp_endpoint_t ep)
    : m_sock(get_io())
    , m_ep(ep) {
}

std::vector<uint8_t> hosted_udp_transport::get_buffer() {
    return std::vector<uint8_t>(2048);
}

std::vector<uint8_t> hosted_udp_transport::send_receive(tos::span<uint8_t> buffer) {
    m_sock.send_to(buffer, m_ep);
    std::vector<uint8_t> res(2048);
    udp_endpoint_t from_ep;
    auto resp = m_sock.receive_from(res, from_ep);
    res.resize(force_get(resp).size());
    return res;
}
} // namespace tos::ae