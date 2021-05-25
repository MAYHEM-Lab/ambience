#include <tos/ae/transport/lwip/udp.hpp>
#include <tos/debug/log.hpp>

namespace tos::ae {
udp_transport::udp_transport(tos::udp_endpoint_t ep)
    : m_ep{ep} {
    m_sock.attach(*this);
}

void udp_transport::operator()(tos::lwip::events::recvfrom_t,
                               tos::lwip::async_udp_socket*,
                               const udp_endpoint_t& from,
                               tos::lwip::buffer&& buf) {
    m_buf = std::move(buf);
    m_wait.up();
}

tos::span<uint8_t> udp_transport::send_receive(tos::span<uint8_t> buffer) {
    m_buf = {};
    m_sock.send_to(buffer, m_ep);
    m_wait.down();
    return m_buf.cur_bucket();
}

std::vector<uint8_t> udp_transport::get_buffer() {
    return std::vector<uint8_t>(1024);
}
} // namespace tos::ae