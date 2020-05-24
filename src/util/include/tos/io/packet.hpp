#pragma once

#include <algorithm>
#include <cstdint>
#include <tos/flags.hpp>
#include <tos/intrusive_list.hpp>
#include <tos/span.hpp>
#include <tos/semaphore.hpp>

namespace tos::io {
enum class packet_flags : uint8_t
{
    owning = 1,
    constant = 2
};

struct packet : list_node<packet> {
public:
    explicit packet(size_t size)
        : m_size(size) {
        if (size != 0) {
            m_ptr = new uint8_t[size];
            set_flags(packet_flags::owning);
        }
    }

    explicit packet(span<uint8_t> data)
        : m_size(data.size())
        , m_ptr(data.data()) {
    }

    size_t size() const {
        return m_size & 0x3FF;
    }

    packet_flags flags() const {
        return packet_flags(m_size >> 10);
    }

    span<uint8_t> data() {
        return {m_ptr, size()};
    }

    span<const uint8_t> data() const {
        return {m_ptr, size()};
    }

    ~packet() {
        if (util::is_flag_set(flags(), packet_flags::owning)) {
            delete[] m_ptr;
        }
    }

private:
    void set_flags(packet_flags flags) {
        m_size = size() | (uint16_t(flags) << 10);
    }

    uint16_t m_size;
    uint8_t* m_ptr;
};
} // namespace tos::io