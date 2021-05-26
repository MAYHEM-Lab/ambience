#include <deque>
#include <memory>
#include <tos/components/allocator.hpp>
#include <tos/gnu_build_id.hpp>
#include <tos/lwip/common.hpp>
#include <tos/lwip/lwip.hpp>
#include <tos/lwip/tcp.hpp>
#include <tos/lwip/tcp_stream.hpp>
#include <tos/mem_stream.hpp>

void http_server() {
    tos::lwip::tcp_socket sock(tos::port_num_t{80});

    tos::mutex backlog_mut;
    tos::semaphore sem{0};
    std::deque<std::unique_ptr<tos::tcp_stream<tos::lwip::tcp_endpoint>>> backlog;
    auto handler = [&](auto& sock, tos::lwip::tcp_endpoint&& ep) {
        tos::lock_guard lg(backlog_mut);
        if (backlog.size() > 20) {
            LOG("Busy!");
            return false;
        }
        backlog.emplace_back(
            std::make_unique<tos::tcp_stream<tos::lwip::tcp_endpoint>>(std::move(ep)));
        sem.up();
        return true;
    };

    int x = 0;
    auto handle_req =
        [&](int id, std::unique_ptr<tos::tcp_stream<tos::lwip::tcp_endpoint>> sockstr) {
            auto begin = tos::lwip::global::system_clock->now();
            std::array<uint8_t, 1024> buf;
            tos::omemory_stream str(buf);
            tos::println(str, "HTTP/1.0 200 Content-type: text/html");
            tos::println(str);
            tos::println(str, "<body>");
            tos::println(str, "<h1>Hello from ambience!</h1>");
            tos::println(str, "Tos build name:", build_id.name(), "<br />");
            tos::println(str, "Tos build id:", build_id.id(), "<br />");
            tos::println(str, "Request num:", x++, "<br />");
            tos::println(str, "Worker id:", id, "<br />");
            tos::println(str, "Backlog size:", backlog.size(), "<br />");
            auto* alloc =
                tos::current_context().get_component<tos::allocator_component>();
            tos::println(str,
                         "Heap memory in use:",
                         alloc->allocator->in_use().value_or(-1),
                         "<br />");
            tos::println(str,
                         "Uptime:",
                         (int)std::chrono::duration_cast<std::chrono::seconds>(
                             tos::lwip::global::system_clock->now().time_since_epoch())
                             .count(),
                         "seconds",
                         "<br />");
            tos::println(str,
                         "Req took:",
                         (int)std::chrono::duration_cast<std::chrono::milliseconds>(
                             tos::lwip::global::system_clock->now() - begin)
                             .count(),
                         "milliseconds",
                         "<br />");
            tos::println(str, "</body>");
            sockstr->write(str.get());
        };

    auto thread = [&](int worker_id, tos::cancellation_token& tok) {
        while (!tok.is_cancelled()) {
            sem.down(tok);
            backlog_mut.lock();
            auto sock = std::move(backlog.front());
            backlog.pop_front();
            backlog_mut.unlock();
            handle_req(worker_id, std::move(sock));
        }
    };

    for (int i = 0; i < 5; ++i) {
        auto& t =
            tos::launch(tos::alloc_stack, thread, i, tos::cancellation_token::system());
        set_name(t, "HTTP Thread Pool Worker");
    }

    sock.async_accept(handler);
}