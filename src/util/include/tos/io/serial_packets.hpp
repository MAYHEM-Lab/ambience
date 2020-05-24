#pragma once

#include "tos/intrusive_list.hpp"

#include <memory>
#include <tos/crc32.hpp>
#include <tos/debug/log.hpp>
#include <tos/expected.hpp>
#include <tos/io/packet.hpp>
#include <tos/mutex.hpp>
#include <tos/semaphore.hpp>

namespace tos::io {
struct port_data : list_node<port_data> {
    explicit port_data(uint16_t port)
        : m_port{port} {
    }
    semaphore m_packets_len{0};
    intrusive_list<packet> m_packets;
    uint16_t m_port;
};

enum class serial_packet_errors
{
    stream_closed,
    bad_magic,
    bad_crc,
    out_of_memory
};

template<class StreamT>
class serial_packets {
public:
    explicit serial_packets(StreamT str, bool write_only = false)
        : m_usart(std::move(str)) {
        if (write_only) {
            return;
        }
        m_thread = &launch(alloc_stack, [this] { thread(); });
        send(0, tos::empty_span<const uint8_t>());
    }

    using streamid_t = int;
    void send(streamid_t streamid, tos::span<const uint8_t> span) {
        tos::lock_guard g{m_write_prot};
        this->m_usart->write(magic_numbers);
        this->m_usart->write(raw_cast<const uint8_t>(tos::monospan(streamid)));
        uint16_t size = span.size();
        this->m_usart->write(raw_cast<const uint8_t>(tos::monospan(size)));
        this->m_usart->write(span);
        uint32_t crc32 = tos::crc32(span);
        this->m_usart->write(raw_cast<const uint8_t>(tos::monospan(crc32)));
    }

    std::unique_ptr<packet> receive(streamid_t streamid) {
        auto str = find_port(streamid);
        if (!str) {
            return nullptr;
        }
        str->m_packets_len.down();
        auto ptr = std::unique_ptr<packet>(&str->m_packets.front());
        str->m_packets.pop_front();
        return ptr;
    }

    void open(streamid_t id) {
        if (find_port(id)) {
            return;
        }

        m_ports.push_back(*new port_data(id));
    }

    void close(streamid_t id) {
        auto port = find_port(id);
        if (!port) {
            return;
        }
        while (!port->m_packets.empty()) {
            auto& packet = port->m_packets.front();
            port->m_packets.pop_front();
            delete &packet;
        }
        m_ports.erase(m_ports.unsafe_find(*port));
        delete port;
    }

private:
    void append_packet(port_data& stream, packet& p) {
        stream.m_packets.push_back(p);
        stream.m_packets_len.up();
    }

    bool m_stop = false;
    void thread() {
        LOG("Serial packets thread running");
        while (!m_stop) {
            auto rd = next_packet();
            if (rd) {
            } else {
                if (force_error(rd) == serial_packet_errors::stream_closed) {
                    m_stop = true;
                }
            }
        }
    }

    using checksum_type = uint32_t;
    constexpr static std::array<uint8_t, 4> magic_numbers = {0x78, 0x9c, 0xc5, 0x45};
    tos::expected<streamid_t, serial_packet_errors> next_packet() {
        for (auto chr : magic_numbers) {
            uint8_t tmp;
            auto read_res = m_usart->read(tos::monospan(tmp));
            if (read_res.empty()) {
                return tos::unexpected(serial_packet_errors::stream_closed);
            }
            if (tmp != chr) {
                return tos::unexpected(serial_packet_errors::bad_magic);
            }
        }

        // LOG_TRACE("Passed magic bytes");

        streamid_t streamid;
        auto read_res = m_usart->read(raw_cast<uint8_t>(tos::monospan(streamid)));
        if (read_res.empty()) {
            return tos::unexpected(serial_packet_errors::stream_closed);
        }

        // LOG_TRACE("Stream id:", streamid);

        uint16_t size;
        read_res = m_usart->read(raw_cast<uint8_t>(tos::monospan(size)));
        if (read_res.empty()) {
            return tos::unexpected(serial_packet_errors::stream_closed);
        }

        // LOG_TRACE("Size:", size);

        auto p = std::make_unique<packet>(size);
        if (!p) {
            LOG_WARN("Could not allocate packet!");
            return tos::unexpected(serial_packet_errors::out_of_memory);
        }
        if (size > 0) {
            read_res = m_usart->read(p->data());
            if (read_res.empty()) {
                // Did not get to read the whole CRC, drop packet
                return tos::unexpected(serial_packet_errors::stream_closed);
            }
        }
        uint32_t crc = crc32(p->data());
        // LOG_TRACE("Computed crc:", crc);

        checksum_type wire_crc;
        read_res = m_usart->read(tos::raw_cast<uint8_t>(tos::monospan(wire_crc)));
        if (read_res.empty()) {
            // Did not get to read the whole CRC, drop packet
            return tos::unexpected(serial_packet_errors::stream_closed);
        }

        // LOG_TRACE("Wire crc:", crc);

        if (wire_crc != crc) {
            // CRC Mismatch, drop packet
            return tos::unexpected(serial_packet_errors::bad_crc);
        }

        if (streamid == 0) {
            // LOG("Received control packet");
            // Control packet
            if (p->size() == 0) {
                send(0, tos::raw_cast(tos::span("HELO")));
            } else if (p->data() == tos::raw_cast(tos::span("HELO"))) {
                // LOG("Received hello reply");
                m_ready = true;
            }
            return streamid;
        }

        auto stream = find_port(streamid);
        if (!stream) {
            LOG_WARN("Received packet from non-existent stream", streamid);
            return streamid;
        }

        append_packet(*stream, *p.release());

        return streamid;
    }

    port_data* find_port(streamid_t port) {
        auto it = std::find_if(this->m_ports.begin(),
                               this->m_ports.end(),
                               [port](auto& stream) { return stream.m_port == port; });
        if (it == this->m_ports.end())
            return nullptr;
        return &(*it);
    }

    bool m_ready = false;
    tos::mutex m_write_prot;
    StreamT m_usart;
    intrusive_list<port_data> m_ports;
    tos::kern::tcb* m_thread;
};
} // namespace tos::io