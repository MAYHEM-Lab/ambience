//
// Created by fatih on 3/9/19.
//

#pragma once

#include <utility> // work around boost missing include bug

#include <boost/asio.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <common/inet/tcp_ip.hpp>
#include <memory>
#include <tos/expected.hpp>
#include <tos/semaphore.hpp>

namespace tos::hosted {
class tcp_socket : public self_pointing<tcp_socket> {
public:
    explicit tcp_socket(std::unique_ptr<boost::asio::ip::tcp::socket> sock)
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

    expected<span<uint8_t>, boost::system::error_code> read(span<uint8_t> buf) {
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
            return unexpected(ec);
        }
        return buf.slice(0, size);
    }

    void close() {
        m_sock->close();
    }

    ~tcp_socket() {
        close();
    }

private:
    semaphore m_read{0};
    semaphore m_write{0};
    std::unique_ptr<boost::asio::ip::tcp::socket> m_sock;
};

class tcp_listener {
public:
    explicit tcp_listener(port_num_t port)
        : m_acceptor(get_io()) {
        m_acceptor.open(boost::asio::ip::tcp::v4());

        boost::asio::socket_base::reuse_address option(true);
        m_acceptor.set_option(option);

        using endpoint_type = boost::asio::ip::tcp::acceptor::endpoint_type;
        endpoint_type endpoint(boost::asio::ip::tcp::v4(), port.port);
        m_acceptor.bind(endpoint);
    }

    expected<void, boost::system::error_code> listen() {
        m_acceptor.listen(4);
        return {};
    }

    expected<std::unique_ptr<tcp_socket>, boost::system::error_code> accept() {
        start_accept();
        m_accept_sem.down();
        return std::make_unique<tcp_socket>(std::move(m_sock));
    }

private:
    void start_accept() {
        m_sock = std::make_unique<boost::asio::ip::tcp::socket>(get_io());
        m_acceptor.async_accept(
            *m_sock, [this](boost::system::error_code ec) { handle_accept_res(ec); });
    }

    void handle_accept_res(boost::system::error_code ec) {
        if (!ec) {
            m_accept_sem.up();
        }
    }

    std::unique_ptr<boost::asio::ip::tcp::socket> m_sock;
    boost::asio::ip::tcp::acceptor m_acceptor;
    semaphore m_accept_sem{0};
};

expected<std::unique_ptr<tcp_socket>, boost::system::error_code>
inline connect(ipv4_addr_t addr, port_num_t port) {
    auto sock = std::make_unique<boost::asio::ip::tcp::socket>(get_io());
    using endpoint_type = boost::asio::ip::tcp::acceptor::endpoint_type;
    boost::asio::ip::address_v4 address(
        {addr.addr[0], addr.addr[1], addr.addr[2], addr.addr[3]});
    endpoint_type endpoint(address, port.port);

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

    return std::make_unique<tcp_socket>(std::move(sock));
}
} // namespace tos::hosted
