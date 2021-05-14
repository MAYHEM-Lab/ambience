#include <tos/detail/poll.hpp>
#include <tos/ae/transport/lwip/host.hpp>

namespace tos::ae {
async_lwip_host::async_lwip_host(const async_service_host& service, tos::port_num_t port)
    : serv{service}
    , sock{port} {
    udp_sock.attach(*this);
    udp_sock.bind(port);
    sock.async_accept(*this);
    tos::launch(tos::alloc_stack, [this] { serve_thread(); });
}

bool async_lwip_host::operator()(tos::lwip::tcp_socket&,
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

void async_lwip_host::operator()(tos::lwip::events::recvfrom_t,
                                 tos::lwip::async_udp_socket*,
                                 const tos::udp_endpoint_t& from,
                                 tos::lwip::buffer&& buf) {
    tos::launch(tos::alloc_stack, [this, buf = std::move(buf), from]() mutable {
        handle_one_req(from, buf);
    });
}

std::vector<uint8_t>
async_lwip_host::read_req(tos::tcp_stream<tos::lwip::tcp_endpoint>& stream) {
    uint16_t len;
    stream.read(tos::raw_cast<uint8_t>(tos::monospan(len)));

    std::vector<uint8_t> buffer(len);
    stream.read(buffer);

    return buffer;
}

void async_lwip_host::handle_one_req(tos::span<uint8_t> req,
                                     lidl::message_builder& response_builder) {
    tos::semaphore exec_sem{0};

    auto coro_runner = [&]() -> tos::Task<bool> {
        auto res = co_await serv.run_message(req, response_builder);
        exec_sem.up();
        co_return res;
    };

    auto j = tos::coro::make_pollable(coro_runner());
    j.run();

    exec_sem.down();
}

void async_lwip_host::handle_one_req(tos::tcp_stream<tos::lwip::tcp_endpoint>& stream) {
    auto req = read_req(stream);
    std::array<uint8_t, 128> resp;
    lidl::message_builder response_builder{resp};

    handle_one_req(req, response_builder);

    uint16_t resp_len = response_builder.get_buffer().size();
    stream.write(tos::raw_cast(tos::monospan(resp_len)));
    stream.write(response_builder.get_buffer());
}

void async_lwip_host::handle_one_req(const tos::udp_endpoint_t& from,
                                     tos::lwip::buffer& buf) {
    if (buf.size() == buf.cur_bucket().size()) {
        auto req = buf.cur_bucket();

        std::array<uint8_t, 128> resp;
        lidl::message_builder response_builder{resp};

        handle_one_req(req, response_builder);

        udp_sock.send_to(response_builder.get_buffer(), from);
    }
}

void async_lwip_host::serve_thread() {
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
} // namespace tos::ae