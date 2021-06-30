#include <tos/ae/transport/lwip/host.hpp>
#include <tos/detail/poll.hpp>

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
    tos::launch(tos::alloc_stack, [this, buf = std::move(buf), from]() mutable {
        handle_one_req(from, buf);
    });
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

namespace {
void handle_req(async_service_host& serv,
                tos::span<uint8_t> req,
                lidl::message_builder& response_builder) {
    tos::semaphore exec_sem{0};

    tos::coro::make_detached(serv.run_message(req, response_builder),
                             tos::make_semaphore_upper(exec_sem));

    exec_sem.down();
}

void handle_req(sync_service_host& serv,
                tos::span<uint8_t> req,
                lidl::message_builder& response_builder) {
    serv.run_message(req, response_builder);
}
} // namespace

template<class ServiceHost>
void lwip_host<ServiceHost>::handle_one_req(
    tos::tcp_stream<tos::lwip::tcp_endpoint>& stream) {
    auto req = read_req(stream);
    std::array<uint8_t, 128> resp;
    lidl::message_builder response_builder{resp};

    handle_req(serv, req, response_builder);

    uint16_t resp_len = response_builder.get_buffer().size();
    stream.write(tos::raw_cast(tos::monospan(resp_len)));
    stream.write(response_builder.get_buffer());
}

template<class ServiceHost>
void lwip_host<ServiceHost>::handle_one_req(const tos::udp_endpoint_t& from,
                                            tos::lwip::buffer& buf) {
    if (buf.size() == buf.cur_bucket().size()) {
        auto req = buf.cur_bucket();

        std::array<uint8_t, 128> resp;
        lidl::message_builder response_builder{resp};

        handle_req(serv, req, response_builder);

        udp_sock.send_to(response_builder.get_buffer(), from);
    }
}

template<class ServiceHost>
void lwip_host<ServiceHost>::serve_thread() {
    while (!tok->is_cancelled()) {
        if (sem.down(*tok) != tos::sem_ret::normal) {
            return;
        }

        backlog_mut.lock();
        auto sock = std::move(backlog.front());
        backlog.pop_front();
        backlog_mut.unlock();
        handle_one_req(*sock);
    }
}

template class lwip_host<sync_service_host>;
template class lwip_host<async_service_host>;
} // namespace tos::ae