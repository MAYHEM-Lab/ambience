#include <arch/udp.hpp>
#include <tos/semaphore.hpp>

namespace asio = boost::asio;
namespace tos::hosted {
udp_socket::udp_socket(boost::asio::io_service& io)
    : m_sock(std::make_unique<asio::ip::udp::socket>(io)) {
    m_sock->open(asio::ip::udp::v4());
}

expected<void, boost::system::error_code>
udp_socket::send_to(tos::span<const uint8_t> data, const udp_endpoint_t& to) {
    asio::ip::udp::endpoint ep;
    ep.address(asio::ip::address_v4(
        {to.addr.addr[0], to.addr.addr[1], to.addr.addr[2], to.addr.addr[3]}));
    ep.port(to.port.port);
    m_sock->send_to(asio::buffer(data.data(), data.size()), ep);
    return {};
}

expected<span<uint8_t>, boost::system::error_code>
udp_socket::receive_from(tos::span<uint8_t> data, udp_endpoint_t& from) {
    asio::ip::udp::endpoint ep;
    int sz;
    tos::semaphore sem{0};
    m_sock->async_receive_from(
        asio::buffer(data.data(), data.size()),
        ep,
        [&](const boost::system::error_code& error, // Result of operation.
            std::size_t bytes_transferred) {
            sz = bytes_transferred;
            sem.up();
        });
    sem.down();
    from.port.port = ep.port();
    auto from_bytes = ep.address().to_v4().to_bytes();
    std::copy(from_bytes.begin(), from_bytes.end(), from.addr.addr.begin());
    return data.slice(0, sz);
}

expected<void, boost::system::error_code> udp_socket::bind(port_num_t port) {
    asio::ip::udp::endpoint ep;
    ep.address(asio::ip::address_v4());
    ep.port(port.port);

    m_sock->bind(ep);
    return {};
}
} // namespace tos::hosted