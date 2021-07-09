#pragma once

#include <arch/udp.hpp>
#include <tos/ae/service_host.hpp>

namespace tos::ae {
template <class ServiceHost>
class hosted_udp_host {
public:
    hosted_udp_host(const ServiceHost& service, tos::port_num_t port);

private:

    void recv_thread();

    tos::hosted::udp_socket m_sock;
    ServiceHost m_serv;
};

extern template class hosted_udp_host<sync_service_host>;
extern template class hosted_udp_host<async_service_host>;
}