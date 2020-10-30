#pragma once

#include <algorithm>
#include <cstdint>
#include <tos/flags.hpp>
#include <tos/intrusive_list.hpp>
#include <tos/semaphore.hpp>
#include <tos/span.hpp>
#include <tos/intrusive_ptr.hpp>

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
        : m_ptr(data.data())
        , m_size(data.size()) {
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

    explicit operator span<const uint8_t>() const {
        return data();
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

    friend void intrusive_ref(packet* p) {
        p->m_refcnt++;
    }

    friend void intrusive_unref(packet* p) {
        p->m_refcnt--;
        if (p->m_refcnt == 0) {
            delete p;
        }
    }

    uint8_t* m_ptr;
    uint16_t m_size;
    uint8_t m_refcnt = 0;
};

inline span<uint8_t> as_span(packet& p) {
    return p.data();
}

class packet_list {
public:
    void add_packet(intrusive_ptr<packet> packet) {
        m_packets.push_back(*packet);
        intrusive_ref(packet.get());
        m_packets_len.up();
    }

    intrusive_ptr<packet> read_packet() {
        m_packets_len.down();
        auto res = intrusive_ptr<packet>(&m_packets.front());
        m_packets.pop_front();
        return res;
    }

private:
    semaphore m_packets_len{0};
    intrusive_list<packet> m_packets;
};
} // namespace tos::io