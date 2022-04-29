#pragma once

#include "common/inet/tcp_ip.hpp"
#include <algorithm>
#include <csetjmp>
#include <fast_float/fast_float.h>
#include <import_export_generated.hpp>
#include <jsmn.hpp>
#include <jsmn_lidl.hpp>
#include <string_view>
#include <tos/ae/detail/handle_req.hpp>
#include <tos/function_ref.hpp>
#include <tos/lwip/lwip.hpp>
#include <tos/lwip/tcp.hpp>
#include <tos/lwip/tcp_stream.hpp>
#include <type_traits>
#include <unordered_map>

namespace tos::ae {
struct http_endpoint {
    virtual bool handle_request(const jsmn::object_parser& body,
                                tos::function_ref<bool(std::string&&)>) = 0;
    virtual ~http_endpoint() = default;
};

struct http_server : exporter::sync_server {
    lwip::tcp_socket sock;

    explicit http_server(port_num_t port)
        : sock{port} {
    }

    void handle_req(tcp_stream<lwip::tcp_endpoint>& str);

    bool operator()(lwip::tcp_socket& sock, lwip::tcp_endpoint&& ep);

    http_endpoint* endpoint_for_route(std::string_view url);

    std::unordered_map<std::string_view, http_endpoint*> m_routes;

    int64_t number_of_calls() override {
        return 0;
    }

    static http_server* instance();
};

template<class ServiceT>
struct http_rest_exporter : http_endpoint {
    using service_type = typename ServiceT::service_type;

    http_rest_exporter(ServiceT* serv, std::string_view url)
        : m_serv{serv} {
        http_server::instance()->m_routes.emplace(url, this);
        LOG_FORMAT("Exported {} at {}", static_cast<void*>(serv), url);
    }

    bool handle_request(const jsmn::object_parser& body,
                        tos::function_ref<bool(std::string&&)> done) override {
        return handle(body, [&](auto& val) { return done(fmt::to_string(val)); });
    }

    template<class ResponseT>
    bool handle(const jsmn::object_parser& body, ResponseT&& resp) {
        std::vector<uint8_t> buf(1024);
        lidl::message_builder req_mb(buf);

        jmp_buf err;
        if (auto val = setjmp(err); val != 0) {
            return false;
        }

        decltype(auto) call_union_res = tos::ae::detail::try_translate_union<
            typename service_type::wire_types::call_union>(body, req_mb, err);

        std::array<uint8_t, 128> buffer;
        lidl::message_builder mb(buffer);

        tos::ae::sync_run_union(*m_serv, call_union_res, mb);

        const auto& calc_resp =
            lidl::get_root<typename service_type::wire_types::return_union>(
                mb.get_buffer());

        return visit(
            [&]<tos::fixed_string Name>(const auto& val) { return resp(val.ret0()); },
            calc_resp);
    }

    ServiceT* m_serv;
};

http_server* create_http_server(int port);
} // namespace tos::ae
