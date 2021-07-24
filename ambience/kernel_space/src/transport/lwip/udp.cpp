#include <tos/ae/transport/lwip/udp.hpp>
#include <tos/debug/log.hpp>
#include <tos/lwip/common.hpp>

namespace tos::ae {
udp_transport::udp_transport(tos::udp_endpoint_t ep)
    : m_ep{ep} {
    m_sock.attach(*this);
    tos::launch(tos::alloc_stack, [ep, this]() mutable {
        while (true) {
            std::array<uint8_t, 16> buf;
            ep.port.port = 0xFFFE;
            auto res = m_sock.send_to(buf, ep);
            if (res) {
                break;
            }
            tos::this_thread::yield();
        }
    });
}

void udp_transport::operator()(tos::lwip::events::recvfrom_t,
                               tos::lwip::async_udp_socket*,
                               const udp_endpoint_t& from,
                               tos::lwip::buffer&& buf) {
    m_buf = std::move(buf);
    m_wait.up();
}

tos::span<uint8_t> udp_transport::send_receive(tos::span<uint8_t> buffer) {
    auto now = tos::lwip::global::system_clock->now();
    auto res = m_sock.send_to(buffer, m_ep);
    auto after = tos::lwip::global::system_clock->now();
    m_wait.down();
    auto later = tos::lwip::global::system_clock->now();
    return m_buf.cur_bucket();
}

std::vector<uint8_t> udp_transport::get_buffer() {
    return std::vector<uint8_t>(1024);
}

async_udp_transport::async_udp_transport(tos::udp_endpoint_t ep)
    : m_ep{ep} {
    m_sock.attach(*this);
    tos::launch(tos::alloc_stack, [ep, this]() mutable {
        while (true) {
            std::array<uint8_t, 16> buf;
            ep.port.port = 0xFFFE;
            auto res = m_sock.send_to(buf, ep);
            if (res) {
                break;
            }
            tos::this_thread::yield();
        }
    });
}

void async_udp_transport::operator()(tos::lwip::events::recvfrom_t,
                                     tos::lwip::async_udp_socket*,
                                     const udp_endpoint_t& from,
                                     tos::lwip::buffer&& buf) {
    m_buf = std::move(buf);
    m_wait.up();
}

Task<span<uint8_t>> async_udp_transport::send_receive(tos::span<uint8_t> buffer) {
    auto now = tos::lwip::global::system_clock->now();
    auto res = co_await m_sock.async_send_to(buffer, m_ep);
    auto after = tos::lwip::global::system_clock->now();
    co_await m_wait;
    auto later = tos::lwip::global::system_clock->now();
    co_return m_buf.cur_bucket();
}

std::vector<uint8_t> async_udp_transport::get_buffer() {
    return std::vector<uint8_t>(1024);
}
} // namespace tos::ae