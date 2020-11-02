#pragma once

#include <tos/intrusive_ptr.hpp>
#include <tos/io/channel.hpp>
#include <tos/io/packet.hpp>
#include <tos/span.hpp>

namespace tos::io {
// This class implements a lidl-rpc capable transport over an io channel.
class packet_transport {
public:
    explicit packet_transport(intrusive_ptr<any_channel> channel)
        : m_channel{std::move(channel)} {
    }

protected:
    std::vector<uint8_t> get_buffer() {
        // TODO(fatih): un-hardcode this
        return std::vector<uint8_t>(256);
    }

    intrusive_ptr<packet> send_receive(span<const uint8_t> send_buf) {
        m_channel->send(send_buf);
        return m_channel->receive();
    }

    intrusive_ptr<any_channel> m_channel;
};
}