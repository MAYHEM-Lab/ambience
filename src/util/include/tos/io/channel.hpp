#pragma once

#include <tos/span.hpp>
#include <cstdint>
#include <tos/io/packet.hpp>
#include <tos/intrusive_ptr.hpp>

namespace tos::io {
class any_channel : public ref_counted<any_channel> {
public:
    virtual void send(span<const uint8_t>) = 0;
    virtual intrusive_ptr<packet> receive() = 0;

    virtual ~any_channel() = default;
};
}