#pragma once

#include "tos/intrusive_list.hpp"

#include <memory>
#include <tos/crc32.hpp>
#include <tos/debug/log.hpp>
#include <tos/expected.hpp>
#include <tos/io/channel.hpp>
#include <tos/io/packet.hpp>
#include <tos/mutex.hpp>
#include <tos/semaphore.hpp>

namespace tos::io {
struct port_data : list_node<port_data> {
    explicit port_data(uint16_t port)
        : m_port{port} {
    }
    packet_list m_packets;
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
class serial_packets_channel;

template<class StreamT>
class serial_packets {
public:
    explicit serial_packets(StreamT str, bool write_only = false)
        : m_usart(std::move(str)) {
        if (write_only) {
            return;
        }
        m_thread = &launch(alloc_stack, [this] { thread(); });
    }

    using streamid_t = int;
    void send(streamid_t streamid, tos::span<const uint8_t> span) {
        tos::lock_guard g{m_write_prot};
        this->m_usart->write(magic_numbers);
        this->m_usart->write(raw_cast<const uint8_t>(tos::monospan(streamid)));
        uint16_t size = span.size();
        this->m_usart->write(raw_cast<const uint8_t>(tos::monospan(size)));
        if (size != 0) {
            this->m_usart->write(span);
        }
        uint32_t crc32 = tos::crc32(span);
        this->m_usart->write(raw_cast<const uint8_t>(tos::monospan(crc32)));
    }

    intrusive_ptr<packet> receive(streamid_t streamid) {
        auto str = find_port(streamid);
        if (!str) {
            return nullptr;
        }
        return str->m_packets.read_packet();
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

    intrusive_ptr<serial_packets_channel<StreamT>> get_channel(int stream);

private:
    void append_packet(port_data& stream, intrusive_ptr<packet> p) {
        stream.m_packets.add_packet(std::move(p));
    }

    bool m_stop = false;
    void thread() {
        send(0, tos::empty_span<const uint8_t>());
//        LOG("Serial packets thread running");
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
                // LOG_WARN("Failed magic bytes");
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

        auto p = make_intrusive<packet>(size);
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

        append_packet(*stream, std::move(p));

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


template<class StreamT>
class serial_packets_channel : public any_channel {
public:
    serial_packets_channel(serial_packets<StreamT>& packets, int stream)
        : m_packets{&packets}
        , m_stream{stream} {
    }

    void send(span<const uint8_t> span) override {
        m_packets->send(m_stream, span);
    }

    intrusive_ptr<packet> receive() override {
        return m_packets->receive(m_stream);
    }

private:
    serial_packets<StreamT>* m_packets;
    int m_stream;
};

template<class StreamT>
intrusive_ptr<serial_packets_channel<StreamT>>
serial_packets<StreamT>::get_channel(int stream) {
    this->open(stream);
    return make_intrusive<serial_packets_channel<StreamT>>(*this, stream);
}
} // namespace tos::io