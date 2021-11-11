#include "common/inet/tcp_ip.hpp"
#include "lidlrt/vector.hpp"
#include "tos/compiler.hpp"
#include "tos/detail/coro.hpp"
#include "tos/lwip/lwip.hpp"
#include "tos/semaphore.hpp"
#include <functional>
#include <list>
#include <networking_generated.hpp>
#include <tos/lwip/udp.hpp>

namespace tos::services {
namespace {
constexpr udp_endpoint_t convert_endpoint(const tos::services::udpv4_endpoint& ep) {
    return udp_endpoint_t{.addr = {ep.addr().addr()}, .port = {ep.port().port()}};
}

constexpr tos::services::udpv4_endpoint convert_endpoint(const udp_endpoint_t& ep) {
    lidl::array<uint8_t, 4> ip_arr;
    std::copy(ep.addr.addr.begin(), ep.addr.addr.end(), ip_arr.begin());
    const tos::services::ipv4_addr ip{ip_arr};
    return tos::services::udpv4_endpoint{ip, tos::services::ip_port{ep.port.port}};
}

template<class T>
class channel {
public:
    void send(T t) {
        m_list.push_back(std::move(t));
        m_sem.up();
    }

    template<class... Us>
    void emplace(Us&&... us) {
        m_list.emplace_back(std::forward<Us>(us)...);
        m_sem.up();
    }

    tos::Task<T> receive() {
        co_await m_sem;
        auto top = std::move(m_list.front());
        m_list.pop_front();
        co_return top;
    }

    std::list<T> m_list;
    tos::semaphore m_sem{0};
};

struct impl : udp_socket::async_server {
    impl() {
        m_sock.attach(*this);
    }

    void operator()(lwip::events::recvfrom_t,
                    auto&&,
                    udp_endpoint_t from,
                    lwip::buffer&& buf) {
        tos::debug::log("Got packet");
        queue.emplace(from, std::move(buf));
    }

    explicit impl(uint16_t port)
        : impl() {
        m_sock.bind(port_num_t{port});
    }

    tos::Task<bool> send_to(tos::span<uint8_t> data,
                            const tos::services::udpv4_endpoint& to) override {
        co_await m_sock.async_send_to(data, convert_endpoint(to));
        co_return true;
    }

    tos::Task<const tos::services::recvfrom_res&>
    recv_from(::lidl::message_builder& response_builder) override {
        auto [ep, buffer] = co_await queue.receive();
        auto& v = lidl::create_vector_sized<uint8_t>(response_builder, buffer.size());
        buffer.read(v.span());
        auto conv_ep = convert_endpoint(ep);
        co_return lidl::create<recvfrom_res>(response_builder, v, conv_ep);
    }

    channel<std::pair<udp_endpoint_t, lwip::buffer>> queue;
    lwip::async_udp_socket m_sock;
};
} // namespace
} // namespace tos::services

tos::Task<tos::services::udp_socket::async_server*> init_lwip_udp_socket() {
    co_return new tos::services::impl;
}

tos::Task<tos::services::udp_socket::async_server*> init_lwip_udp_server(int port) {
    co_return new tos::services::impl(port);
}