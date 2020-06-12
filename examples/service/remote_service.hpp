#pragma once

#include <cstdint>
#include <tos/intrusive_ptr.hpp>
#include <tos/io/channel.hpp>
#include <tos/io/packet.hpp>
#include <tos/span.hpp>

namespace lidl {
class remote_service {
public:
    explicit remote_service(tos::intrusive_ptr<tos::io::any_channel> channel)
        : m_channel{std::move(channel)} {
    }

protected:
    tos::intrusive_ptr<tos::io::packet> send_receive(tos::span<const uint8_t> send_buf) {
        m_channel->send(send_buf);
        return m_channel->receive();
    }

    tos::intrusive_ptr<tos::io::any_channel> m_channel;
};
} // namespace lidl