#pragma once

#include <arch/udp.hpp>
#include <import_export_generated.hpp>
#include <lidlrt/transport/common.hpp>
#include <tos/ae/importer.hpp>

namespace tos::ae {
struct hosted_udp_transport : lidl::verbatim_transform {
    hosted_udp_transport(tos::udp_endpoint_t ep);

    std::vector<uint8_t> get_buffer();
    std::vector<uint8_t> send_receive(tos::span<uint8_t> buffer);

private:
    tos::hosted::udp_socket m_sock;
    tos::udp_endpoint_t m_ep;
};

using udp_import_args = udp_endpoint_t;

struct hosted_udp_importer : importer::sync_server {
    template<class Service>
    typename Service::sync_server* import_service(const udp_import_args& args) {
        return new typename Service::template stub_client<hosted_udp_transport>(args);
    }

    int64_t number_of_calls() override {
        return 0;
    }
};
} // namespace tos::ae