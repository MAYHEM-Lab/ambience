#include <tos/ae/detail/handle_req.hpp>
#include <tos/ae/transport/lwip/host.hpp>
#include <tos/debug/log.hpp>
#include <tos/detail/poll.hpp>
#include <tos/lwip/common.hpp>

namespace tos::ae {
template<class ServiceHost>
lwip_host<ServiceHost>::lwip_host(const ServiceHost& service, tos::port_num_t port)
    : serv{service}
    , sock{port} {
    udp_sock.attach(*this);
    udp_sock.bind(port);
    sock.async_accept(*this);
    set_name(tos::launch(tos::alloc_stack, [this] { serve_thread(); }), "Serve thread");
}

template<class ServiceHost>
bool lwip_host<ServiceHost>::operator()(tos::lwip::tcp_socket&,
                                        tos::lwip::tcp_endpoint&& client) {
    tos::lock_guard lg(backlog_mut);
    if (backlog.size() > 20) {
        LOG("Busy!");
        return false;
    }
    backlog.emplace_back(
        std::make_unique<tos::tcp_stream<tos::lwip::tcp_endpoint>>(std::move(client)));
    sem.up();
    return true;
}

template<class ServiceHost>
void lwip_host<ServiceHost>::operator()(tos::lwip::events::recvfrom_t,
                                        tos::lwip::async_udp_socket*,
                                        const tos::udp_endpoint_t& from,
                                        tos::lwip::buffer&& buf) {
    tos::coro::make_detached(async_handle_one_req(from, std::move(buf)));
}

template<class ServiceHost>
std::vector<uint8_t>
lwip_host<ServiceHost>::read_req(tos::tcp_stream<tos::lwip::tcp_endpoint>& stream) {
    uint16_t len;
    stream.read(tos::raw_cast<uint8_t>(tos::monospan(len)));

    std::vector<uint8_t> buffer(len);
    stream.read(buffer);

    return buffer;
}

template<class ServiceHost>
void lwip_host<ServiceHost>::handle_one_req(
    tos::tcp_stream<tos::lwip::tcp_endpoint>& stream) {
    auto req = read_req(stream);
    std::array<uint8_t, 2048> resp;
    lidl::message_builder response_builder{resp};

    sync_run_message(serv, req, response_builder);

    uint16_t resp_len = response_builder.get_buffer().size();
    stream.write(tos::raw_cast(tos::monospan(resp_len)));
    stream.write(response_builder.get_buffer());
}

template<class ServiceHost>
void lwip_host<ServiceHost>::handle_one_req(const tos::udp_endpoint_t& from,
                                            tos::lwip::buffer& buf) {
    if (buf.size() == buf.cur_bucket().size()) {
        auto req = buf.cur_bucket();

        std::array<uint8_t, 2048> resp;
        lidl::message_builder response_builder{resp};

        [[maybe_unused]] auto now = lwip::global::system_clock->now();
        sync_run_message(serv, req, response_builder);
        [[maybe_unused]] auto end = lwip::global::system_clock->now();
        udp_sock.send_to(response_builder.get_buffer(), from);
    }
}

template<class ServiceHost>
tos::Task<void> lwip_host<ServiceHost>::async_handle_one_req(tos::udp_endpoint_t from,
                                                             tos::lwip::buffer buf) {
    if (buf.size() == buf.cur_bucket().size()) {
        auto req = buf.cur_bucket();

        std::array<uint8_t, 1024> resp;
        lidl::message_builder response_builder{resp};

        [[maybe_unused]] auto now = lwip::global::system_clock->now();
        co_await async_run_message(serv, req, response_builder);
        [[maybe_unused]] auto end = lwip::global::system_clock->now();
        co_await udp_sock.async_send_to(response_builder.get_buffer(), from);
    }
}

template<class ServiceHost>
void lwip_host<ServiceHost>::serve_thread() {
    while (!tok->is_cancelled()) {
        if (sem.down(*tok) != tos::sem_ret::normal) {
            return;
        }

        while (!m_async_responses.empty()) {
            auto& info = m_async_responses.front();
            m_async_responses.pop_front();
            udp_sock.send_to(info.buf, info.to);
            info.done.up();
        }

        backlog_mut.lock();
        while (!backlog.empty()) {
            auto sock = std::move(backlog.front());
            backlog.pop_front();
            handle_one_req(*sock);
        }
        backlog_mut.unlock();
    }
}

template class lwip_host<sync_service_host>;
template class lwip_host<async_service_host>;
} // namespace tos::ae