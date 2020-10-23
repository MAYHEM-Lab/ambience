#pragma once

#include <cstdint>
#include <tos/intrusive_ptr.hpp>
#include <tos/io/channel.hpp>
#include <tos/io/packet.hpp>
#include <tos/span.hpp>
#include <lidl/service.hpp>

class packet_transport {
public:
    explicit packet_transport(tos::intrusive_ptr<tos::io::any_channel> channel)
        : m_channel{std::move(channel)} {
    }

protected:
    std::vector<uint8_t> get_buffer() {
        return std::vector<uint8_t>(256);
    }

    tos::intrusive_ptr<tos::io::packet> send_receive(tos::span<const uint8_t> send_buf) {
        m_channel->send(send_buf);
        return m_channel->receive();
    }

    tos::intrusive_ptr<tos::io::any_channel> m_channel;
};

namespace tos {
inline tos::span<uint8_t> as_span(intrusive_ptr<io::packet>& packet) {
    return packet->data();
}
}

namespace std {
inline tos::span<uint8_t> as_span(vector<uint8_t>& v) {
    return v;
}
}
