#include <tos/ae/transport/hosted/udp_host.hpp>
#include <tos/cancellation_token.hpp>
#include <tos/detail/poll.hpp>
#include <tos/platform.hpp>
#include <tos/semaphore.hpp>

namespace tos::ae {
template<class ServiceHost>
hosted_udp_host<ServiceHost>::hosted_udp_host(const ServiceHost& service,
                                              tos::port_num_t port)
    : m_sock(get_io())
    , m_serv{service} {
    ensure(m_sock.bind(port));
    tos::launch(tos::alloc_stack, [this] { recv_thread(); });
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
void hosted_udp_host<ServiceHost>::recv_thread() {
    while (!tos::cancellation_token::system().is_cancelled()) {
        std::vector<uint8_t> buf(4096);
        udp_endpoint_t from_ep;
        if (auto read_res = m_sock.receive_from(buf, from_ep)) {
            tos::launch(tos::alloc_stack,
                        [this, req = force_get(read_res), buf = std::move(buf), from_ep] {
                            std::vector<uint8_t> resp(4096);
                            lidl::message_builder response_builder(resp);
                            handle_req(m_serv, req, response_builder);

                            m_sock.send_to(response_builder.get_buffer(), from_ep);
                        });
        }
    }
}

template class hosted_udp_host<sync_service_host>;
template class hosted_udp_host<async_service_host>;
} // namespace tos::ae