#pragma once

#include <common/inet/tcp_ip.hpp>
#include <deque>
#include <tos/ae/service_host.hpp>
#include <tos/lwip/tcp.hpp>
#include <tos/lwip/tcp_stream.hpp>
#include <tos/lwip/udp.hpp>
#include <tos/span.hpp>
#include <vector>

namespace tos::ae {
struct async_lwip_host {
    async_lwip_host(const async_service_host& service, tos::port_num_t port);

    // Acceptor
    bool operator()(tos::lwip::tcp_socket&, tos::lwip::tcp_endpoint&& client);

    void operator()(tos::lwip::events::recvfrom_t,
                    tos::lwip::async_udp_socket*,
                    const tos::udp_endpoint_t& from,
                    tos::lwip::buffer&& buf);

private:
    static std::vector<uint8_t>
    read_req(tos::tcp_stream<tos::lwip::tcp_endpoint>& stream);

    void handle_one_req(tos::span<uint8_t> req, lidl::message_builder& response_builder);

    void handle_one_req(tos::tcp_stream<tos::lwip::tcp_endpoint>& stream);

    void handle_one_req(const tos::udp_endpoint_t& from, tos::lwip::buffer& buf);

    void serve_thread();

    tos::cancellation_token* tok = &tos::cancellation_token::system();

    tos::mutex backlog_mut;
    tos::semaphore sem{0};
    std::deque<std::unique_ptr<tos::tcp_stream<tos::lwip::tcp_endpoint>>> backlog;

    tos::ae::async_service_host serv;
    tos::lwip::tcp_socket sock;
    tos::lwip::async_udp_socket udp_sock;
};

} // namespace tos::ae