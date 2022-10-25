//
// Created by fatih on 3/9/19.
//

#pragma once

#include <utility> // work around boost missing include bug

#include <boost/asio.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <common/inet/tcp_ip.hpp>
#include <iostream>
#include <memory>
#include <tos/expected.hpp>
#include <tos/semaphore.hpp>

namespace tos::hosted {
class unix_socket : public self_pointing<unix_socket> {
public:
    explicit unix_socket(
        std::unique_ptr<boost::asio::local::stream_protocol::socket> sock)
        : m_sock{std::move(sock)} {
    }

    expected<int, boost::system::error_code> write(span<const uint8_t> buf) {
        boost::asio::async_write(
            *m_sock,
            boost::asio::buffer(buf.data(), buf.size()),
            [this](boost::system::error_code, std::size_t) { m_write.up(); });
        m_write.down();
        return buf.size();
    }

    span<uint8_t> read(span<uint8_t> buf) {
        int size = 0;
        boost::system::error_code ec;
        boost::asio::async_read(*m_sock,
                                boost::asio::buffer(buf.data(), buf.size()),
                                [&](boost::system::error_code e, std::size_t len) {
                                    size = len;
                                    ec = e;
                                    m_read.up();
                                });
        m_read.down();
        if (ec) {
            LOG_WARN("Error", ec.message());
        }
        return buf.slice(0, size);
    }

    void close() {
        m_sock->close();
    }

    ~unix_socket() {
        close();
    }

private:
    semaphore m_read{0};
    semaphore m_write{0};
    std::unique_ptr<boost::asio::local::stream_protocol::socket> m_sock;
};

class unix_listener {
public:
    explicit unix_listener(std::string path)
        : m_acceptor(get_io(), boost::asio::local::stream_protocol::endpoint(path)) {
    }

    expected<void, boost::system::error_code> listen() {
        m_acceptor.listen(4);
        return {};
    }

    expected<std::unique_ptr<unix_socket>, boost::system::error_code> accept() {
        start_accept();
        m_accept_sem.down();
        return std::make_unique<unix_socket>(std::move(m_sock));
    }

private:
    void start_accept() {
        m_sock = std::make_unique<boost::asio::local::stream_protocol::socket>(get_io());
        m_acceptor.async_accept(
            *m_sock, [this](boost::system::error_code ec) { handle_accept_res(ec); });
    }

    void handle_accept_res(boost::system::error_code ec) {
        if (!ec) {
            m_accept_sem.up();
        }
    }

    std::unique_ptr<boost::asio::local::stream_protocol::socket> m_sock;
    boost::asio::local::stream_protocol::acceptor m_acceptor;
    semaphore m_accept_sem{0};
};

inline expected<std::unique_ptr<unix_socket>, boost::system::error_code>
connect(const std::string& path) {
    auto sock = std::make_unique<boost::asio::local::stream_protocol::socket>(get_io());
    using endpoint_type = boost::asio::local::stream_protocol::acceptor::endpoint_type;
    endpoint_type endpoint(path);

    boost::system::error_code ec;
    semaphore wait_sem{0};
    sock->async_connect(endpoint, [&](boost::system::error_code e) {
        ec = e;
        wait_sem.up();
    });
    wait_sem.down();

    if (ec) {
        return unexpected(ec);
    }

    return std::make_unique<unix_socket>(std::move(sock));
}
} // namespace tos::hosted
