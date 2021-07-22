#include <tos/ae/transport/hosted/unix_domain.hpp>
#include <tos/ae/detail/handle_req.hpp>

namespace tos::ae {
std::vector<uint8_t> unix_domain_importer::transport::get_buffer() {
    return std::vector<uint8_t>(4096);
}

std::vector<uint8_t>
unix_domain_importer::transport::send_receive(tos::span<uint8_t> buffer) {
    m_sock_cache_sem.down();

    auto& state = m_sock_cache.front();
    m_sock_cache.pop_front();

    uint16_t sz = buffer.size();
    state.m_sock->write(raw_cast(monospan(sz)));
    state.m_sock->write(buffer);

    state.m_sock->read(raw_cast<uint8_t>(monospan(sz)));
    std::vector<uint8_t> res(size_t{sz});
    state.m_sock->read(res);

    m_sock_cache.push_back(state);
    m_sock_cache_sem.up();

    return res;
}

template<class ServT>
struct unix_export;

template<class ServT>
struct socket_handler {
    socket_handler(unix_export<ServT>& exporter,
                   std::unique_ptr<hosted::unix_socket> sock)
        : m_export{&exporter}
        , m_sock(std::move(sock)) {
        tos::launch(tos::alloc_stack, [this] { listen_thread(); });
    }

    void listen_thread();

    unix_export<ServT>* m_export;
    std::unique_ptr<hosted::unix_socket> m_sock;
};

template<class ServT>
void socket_handler<ServT>::listen_thread() {
    while (!tos::cancellation_token::system().is_cancelled()) {
        uint16_t sz;
        m_sock->read(raw_cast<uint8_t>(monospan(sz)));
        std::vector<uint8_t> req(size_t{sz});
        m_sock->read(req);

        std::array<uint8_t, 4096> response_buffer;
        lidl::message_builder response_builder(response_buffer);

        sync_run_message(m_export->m_serv, req, response_builder);

        sz = response_builder.get_buffer().size();
        m_sock->write(raw_cast(monospan(sz)));
        m_sock->write(response_builder.get_buffer());
    }
}

template<class ServT>
void unix_export<ServT>::accept_thread() {
    while (!tos::cancellation_token::system().is_cancelled()) {
        auto client_sock = m_listener->accept();
        if (client_sock) {
            tos::debug::info("Unix domain exporter accepted a session");
            new socket_handler(*this, std::move(force_get(client_sock)));
        } else {
            tos::debug::error("Unix domain socket accept returned an error!",
                              force_error(client_sock).message());
        }
    }
}

void unix_domain_exporter::export_service(const sync_service_host& host,
                                          const export_args& args) {
    new unix_export<sync_service_host>(
        host, static_cast<const unix_domain_export_args&>(args).path);
}

void unix_domain_exporter::export_service(const async_service_host& host,
                                          const export_args& args) {
    new unix_export<async_service_host>(
        host, static_cast<const unix_domain_export_args&>(args).path);
}

template struct unix_export<sync_service_host>;
template struct unix_export<async_service_host>;
} // namespace tos::ae