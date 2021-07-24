#pragma once

#include <common/inet/tcp_ip.hpp>
#include <tos/lwip/lwip.hpp>
#include <tos/lwip/udp.hpp>
#include <vector>
#include <lidlrt/transport/common.hpp>
#include <tos/ae/importer.hpp>

namespace tos::ae {
struct udp_transport : lidl::verbatim_transform {
public:
    udp_transport(tos::udp_endpoint_t ep);

    std::vector<uint8_t> get_buffer();
    tos::span<uint8_t> send_receive(tos::span<uint8_t> buffer);

    void operator()(tos::lwip::events::recvfrom_t,
                    tos::lwip::async_udp_socket*,
                    const tos::udp_endpoint_t& from,
                    tos::lwip::buffer&& buf);

private:
    tos::semaphore m_wait{0};
    tos::lwip::buffer m_buf;
    tos::udp_endpoint_t m_ep;
    tos::lwip::async_udp_socket m_sock;
};

struct async_udp_transport : lidl::verbatim_transform {
public:
    async_udp_transport(tos::udp_endpoint_t ep);

    std::vector<uint8_t> get_buffer();
    tos::Task<tos::span<uint8_t>> send_receive(tos::span<uint8_t> buffer);

    void operator()(tos::lwip::events::recvfrom_t,
                    tos::lwip::async_udp_socket*,
                    const tos::udp_endpoint_t& from,
                    tos::lwip::buffer&& buf);

private:
    tos::semaphore m_wait{0};
    tos::lwip::buffer m_buf;
    tos::udp_endpoint_t m_ep;
    tos::lwip::async_udp_socket m_sock;
};

using udp_import_args = udp_endpoint_t;

struct lwip_udp_importer : importer::sync_server {
    template<class Service>
    typename Service::sync_server* import_service(const udp_import_args& args) {
        return new typename Service::template stub_client<udp_transport>(args);
    }

    int64_t number_of_calls() override {
        return 0;
    }
};
} // namespace tos::ae