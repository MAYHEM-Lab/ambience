#pragma once

#include <boost/asio/ip/udp.hpp>
#include <common/inet/tcp_ip.hpp>
#include <cstdint>
#include <tos/expected.hpp>
#include <tos/self_pointing.hpp>
#include <tos/span.hpp>

namespace tos::hosted {
class udp_socket : public self_pointing<udp_socket> {
public:
    udp_socket(boost::asio::io_service& io);

    expected<void, boost::system::error_code> send_to(tos::span<const uint8_t> data,
                                                      const udp_endpoint_t& to);

    expected<span<uint8_t>, boost::system::error_code>
    receive_from(tos::span<uint8_t> data, udp_endpoint_t& from);

private:
    std::unique_ptr<boost::asio::ip::udp::socket> m_sock;
};

} // namespace tos::hosted