#include "fmt/format.h"
#include "jsmn.h"
#include "lidlrt/buffer.hpp"
#include "lidlrt/builder.hpp"
#include "lidlrt/meta.hpp"
#include "lidlrt/structure.hpp"
#include "tos/ae/detail/handle_req.hpp"
#include "tos/debug/assert.hpp"
#include "tos/ft.hpp"
#include "tos/function_ref.hpp"
#include "tos/meta/types.hpp"
#include "tos/utility.hpp"
#include <calc_generated.hpp>
#include <charconv>
#include <fast_float/fast_float.h>
#include <httpparser/httprequestparser.h>
#include <jsmn.hpp>
#include <lidlrt/concepts.hpp>
#include <lidlrt/service.hpp>
#include <memory>
#include <string>
#include <string_view>
#include <tos/ae/registry.hpp>
#include <tos/ae/transport/http/host.hpp>
#include <tos/detail/poll.hpp>
#include <tos/lwip/tcp.hpp>
#include <tos/lwip/tcp_stream.hpp>
#include <type_traits>
#include <unordered_map>

tos::ae::registry_base& get_registry();
namespace tos::ae {
bool http_server::operator()(lwip::tcp_socket& sock, lwip::tcp_endpoint&& ep) {
    LOG("Got connection");

    auto str = std::make_unique<tcp_stream<lwip::tcp_endpoint>>(std::move(ep));
    str->set_buffer_mode();
    tos::launch(tos::alloc_stack,
                [this, str = std::move(str)]() mutable { handle_req(*str); });
    return true;
}

http_endpoint* http_server::endpoint_for_route(std::string_view url) {
    auto it = m_routes.find(url);
    if (it == m_routes.end()) {
        return nullptr;
    }
    return it->second;
};

void http_server::handle_req(tcp_stream<lwip::tcp_endpoint>& str) {
    httpparser::HttpRequestParser parser;
    httpparser::Request req;

    while (!str->disconnected()) {
        auto buf_res = str->sync_read_buffer();
        if (!buf_res) {
            LOG_ERROR("Err, return");
            return;
        }
        auto& buf = force_get(buf_res);

        LOG("Got buffer", buf.cur_bucket().size());
        auto parse_res = parser.parse(req,
                                      (const char*)buf.cur_bucket().begin(),
                                      (const char*)buf.cur_bucket().end());
        if (parse_res == httpparser::HttpRequestParser::ParsingIncompleted) {
            LOG("Incomplete");
            continue;
        }
        if (parse_res == httpparser::HttpRequestParser::ParsingError) {
            tos::debug::error("HTTP Request error!");
            return;
        }

        tos::debug::log(req.method, req.uri, req.content.size());

        auto ep = endpoint_for_route(req.uri);

        std::string_view status = "400 Bad Request";

        if (!ep) {
            status = "404 Not Found";
        }

        std::string resp_string;

        if (ep && !req.content.empty()) {
            std::string_view req_body{req.content.data(), req.content.size()};
            auto toks = jsmn::parse(req_body);
            Assert(toks.has_value());

            jsmn::parser p{.tokens = *toks, .body = req_body};

            jsmn::object_parser obj_p{p};

            auto done_cb = [&](std::string&& val) {
                resp_string = std::move(val);
                return true;
            };

            auto res = ep->handle_request(
                obj_p, tos::function_ref<bool(std::string &&)>(done_cb));

            if (res) {
                status = "200 OK";
            }
        }

        uint8_t crlf[] = "\r\n";
        auto header_string =
            fmt::format(FMT_COMPILE("HTTP/1.0 {}\r\nContent-type: text/plain; "
                                    "charset=utf-8\r\nConnection: close\r\n\r\n"),
                        status);
        str.write(tos::span<const uint8_t>(
            reinterpret_cast<const uint8_t*>(header_string.data()),
            header_string.size()));
        str.write(tos::span<const uint8_t>(
            reinterpret_cast<const uint8_t*>(resp_string.data()), resp_string.size()));
        str.write(tos::span(crlf).pop_back());

        return;
    }
}

namespace {
http_server* g_instance;
}

http_server* create_http_server(int port) {
    auto impl = new http_server{port_num_t{static_cast<uint16_t>(port)}};
    LOG("Accepting");
    impl->sock.async_accept(*impl);
    return impl;
}

http_server* http_server::instance() {
    if (!g_instance) [[unlikely]] {
        g_instance = create_http_server(80);
    }
    return g_instance;
}
} // namespace tos::ae
