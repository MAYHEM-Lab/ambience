//
// Created by fatih on 3/15/20.
//

#include <arch/cpp_clocks.hpp>
#include <arch/drivers.hpp>
#include <tos/debug/dynamic_log.hpp>
#include <tos/debug/log.hpp>
#include <tos/debug/sinks/clock_adapter.hpp>
#include <tos/debug/sinks/serial_sink.hpp>
#include <tos/io/serial_multiplexer.hpp>
#include <tos/streams.hpp>

namespace asio = boost::asio;

namespace tos::tdb {
template<class StreamT>
class channel_owner : tos::non_copy_movable {
public:
    explicit channel_owner(StreamT str)
        : m_str{std::move(str)} {
        m_read_thread = &tos::launch(tos::alloc_stack, [this] { read_run(); });
        m_listen_thread = &tos::launch(tos::alloc_stack, [this] { listen_run(); });
    }

    void cancel() {
        m_cancel = true;
    }

private:
    void read_run() {
        while (!m_cancel) {
            std::array<uint8_t, 1> buffer;
            auto buf = m_str->read(buffer);
            tos::lock_guard g{m_socks_lock};
            for (auto& s : m_unix_socks) {
                s->write(buf);
            }
        }
    }

    template<class ClientStreamT>
    void write_run(ClientStreamT& sock) {
        auto i = m_str->get_stream_id();
        while (!m_cancel) {
            std::array<uint8_t, 1> buffer;
            auto buf = sock->read(buffer);
            if (buf.empty()) {
                LOG_INFO("Socket closed, removing", i);
                tos::lock_guard g{m_socks_lock};
                m_unix_socks.erase(std::remove_if(
                    m_unix_socks.begin(), m_unix_socks.end(), [&sock](auto& ptr) {
                        return ptr.get() == &sock;
                    }));
                return;
            }
            LOG_TRACE("Sending ", buf.size(), " bytes to ", i);
            m_str->write(buf);
            LOG_TRACE("Sent ", buf.size(), " bytes to ", i);
        }
    }

    void listen_run() {
        auto i = m_str->get_stream_id();
        using namespace std::string_literals;
        auto path = "/tmp/tdb/channel"s + std::to_string(i);
        ::unlink(path.c_str());
        auto sock = tos::x86::unix_listener(path);
        sock.listen();

        while (!m_cancel) {
            LOG_INFO("Accepting for", i);
            auto res = sock.accept();
            LOG_INFO("Accepted for", i);
            tos::lock_guard g{m_socks_lock};
            m_unix_socks.emplace_back(std::move(force_get(res)));
            tos::launch(tos::alloc_stack, [this] { write_run(*m_unix_socks.back()); });
        }
    }

    tos::kern::tcb* m_read_thread;
    tos::kern::tcb* m_listen_thread;

    bool m_cancel = false;
    StreamT m_str;

    tos::mutex m_socks_lock;
    std::vector<std::unique_ptr<tos::x86::unix_socket>> m_unix_socks;
    std::vector<tos::kern::tcb*> m_per_sock_thread;
};

class multiplexer {
public:
    void operator()() {
        LOG_INFO("tos tdb");
        LOG_INFO("Version 0.1");
        LOG_INFO("Connecting to device over /dev/ttyACM2");
        LOG_INFO("Opening device");
        m_mux =
            std::make_unique<tos::serial_multiplexer<std::unique_ptr<tos::x86::usart>>>(
                std::make_unique<tos::x86::usart>(
                    get_io(), "/dev/ttyACM2", tos::uart::default_115200),
                std::initializer_list<uint16_t>{0});
        LOG_INFO("Launching channel owner threads");

        for (int i : {0}) {
            m_channels.emplace_back(*m_mux->get_stream(i));
        }

        static tos::semaphore int_sem{0};

        signal(SIGINT, [](int signal) {
             int_sem.up();
        });

        signal(SIGTERM, [](int signal) {
          int_sem.up();
        });

        int_sem.down();
        LOG_INFO("Received termination request, cancelling channels");

        for (auto& channel : m_channels) {
            channel.cancel();
        }

        LOG_INFO("Shutting down");

        tos::this_thread::block_forever();
    }

private:
    std::deque<channel_owner<
        multiplexed_stream<serial_multiplexer<std::unique_ptr<x86::usart>>>>>
        m_channels;
    std::unique_ptr<tos::serial_multiplexer<std::unique_ptr<x86::usart>>> m_mux;
};
} // namespace tos::tdb

void tos_main() {
    tos::debug::set_default_log(
        new tos::debug::detail::any_logger(new tos::debug::clock_sink_adapter{
            tos::debug::serial_sink(tos::x86::stdio{}),
            tos::x86::clock<std::chrono::system_clock>{}}));

    tos::launch(tos::alloc_stack, tos::tdb::multiplexer{});
}