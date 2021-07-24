#pragma once

#include <common/inet/tcp_ip.hpp>
#include <deque>
#include <tos/ae/exporter.hpp>
#include <tos/ae/service_host.hpp>
#include <tos/lwip/tcp.hpp>
#include <tos/lwip/tcp_stream.hpp>
#include <tos/lwip/udp.hpp>
#include <tos/span.hpp>
#include <vector>

namespace tos::ae {
template<class ServiceHost>
struct lwip_host {
    lwip_host(const ServiceHost& service, tos::port_num_t port);

    // Acceptor
    bool operator()(tos::lwip::tcp_socket&, tos::lwip::tcp_endpoint&& client);

    void operator()(tos::lwip::events::recvfrom_t,
                    tos::lwip::async_udp_socket*,
                    const tos::udp_endpoint_t& from,
                    tos::lwip::buffer&& buf);

private:
    static std::vector<uint8_t>
    read_req(tos::tcp_stream<tos::lwip::tcp_endpoint>& stream);

    void handle_one_req(tos::tcp_stream<tos::lwip::tcp_endpoint>& stream);

    void handle_one_req(const tos::udp_endpoint_t& from, tos::lwip::buffer& buf);
    tos::Task<void> async_handle_one_req(tos::udp_endpoint_t from, tos::lwip::buffer buf);

    void serve_thread();

    tos::cancellation_token* tok = &tos::cancellation_token::system();

    tos::mutex backlog_mut;
    tos::semaphore sem{0};
    std::deque<std::unique_ptr<tos::tcp_stream<tos::lwip::tcp_endpoint>>> backlog;

    ServiceHost serv;
    tos::lwip::tcp_socket sock;
    tos::lwip::async_udp_socket udp_sock;

    struct udp_response_info : list_node<udp_response_info> {
        udp_endpoint_t to;
        tos::span<const uint8_t> buf;
        tos::semaphore done{0};
    };

    tos::intrusive_list<udp_response_info> m_async_responses;
};

extern template class lwip_host<sync_service_host>;
extern template class lwip_host<async_service_host>;

struct lwip_exporter : exporter::sync_server {
    int64_t number_of_calls() override {
        return 0;
    }
};
} // namespace tos::ae