#pragma once

#include <tos/crc32.hpp>
#include <tos/expected.hpp>
#include <tos/io/packet.hpp>
#include <tos/mutex.hpp>
#include <tos/span.hpp>
#include <utility>
#include <tos/cancellation_token.hpp>

namespace tos::io {
enum class serial_backend_errors
{
    stream_closed,
    out_of_memory,
    bad_magic,
    bad_crc,
};

template<class StreamT>
class serial_backend {
public:
    using serial_type = StreamT;
    explicit serial_backend(StreamT stream) : m_stream{std::move(stream)} {}

    using streamid_t = int;
    void send(streamid_t streamid, span<const uint8_t> span) {
        lock_guard g{m_write_prot};
        m_stream->write(magic_numbers);
        m_stream->write(raw_cast(monospan(streamid)));
        uint16_t size = span.size();
        m_stream->write(raw_cast(monospan(size)));
        if (size != 0) {
            m_stream->write(span);
        }
        uint32_t crc32 = tos::crc32(span);
        m_stream->write(raw_cast(monospan(crc32)));
    }

    template<class... ReadArgs>
    expected<std::pair<streamid_t, intrusive_ptr<packet>>, serial_backend_errors>
    receive_one(ReadArgs&&... read_args) {
        for (auto chr : magic_numbers) {
            uint8_t tmp;
            auto read_res =
                m_stream->read(monospan(tmp), std::forward<ReadArgs>(read_args)...);
            if (read_res.empty()) {
                return tos::unexpected(serial_backend_errors::stream_closed);
            }
            if (tmp != chr) {
                return tos::unexpected(serial_backend_errors::bad_magic);
            }
        }

        streamid_t streamid;
        auto read_res = m_stream->read(raw_cast<uint8_t>(monospan(streamid)),
                                      std::forward<ReadArgs>(read_args)...);
        if (read_res.empty()) {
            return tos::unexpected(serial_backend_errors::stream_closed);
        }

        uint16_t size;
        read_res = m_stream->read(raw_cast<uint8_t>(monospan(size)),
                                 std::forward<ReadArgs>(read_args)...);
        if (read_res.empty()) {
            return tos::unexpected(serial_backend_errors::stream_closed);
        }

        auto p = make_intrusive<packet>(size);
        if (!p) {
            return tos::unexpected(serial_backend_errors::out_of_memory);
        }

        if (size > 0) {
            read_res = m_stream->read(p->data(), std::forward<ReadArgs>(read_args)...);
            if (read_res.empty()) {
                // Did not get to read the whole CRC, drop packet
                return unexpected(serial_backend_errors::stream_closed);
            }
        }

        uint32_t crc = crc32(p->data());

        uint32_t wire_crc;
        read_res = m_stream->read(raw_cast<uint8_t>(monospan(wire_crc)),
                                 std::forward<ReadArgs>(read_args)...);
        if (read_res.empty()) {
            // Did not get to read the whole CRC, drop packet
            return tos::unexpected(serial_backend_errors::stream_closed);
        }

        if (wire_crc != crc) {
            // CRC Mismatch, drop packet
            return tos::unexpected(serial_backend_errors::bad_crc);
        }

        return std::make_pair(streamid, std::move(p));
    }

private:
    constexpr static std::array<uint8_t, 4> magic_numbers = {0x78, 0x9c, 0xc5, 0x45};
    bool m_ready = false;
    tos::mutex m_write_prot;
    StreamT m_stream;
};

template <class SerialT, class PacketHandlerT>
void poll_packets(serial_backend<SerialT>& transport, cancellation_token& cancel, PacketHandlerT&& handler) {
    while (!cancel.is_cancelled()) {
        auto res = transport.receive_one();
        if (!res) {
            switch (force_error(res)) {
            case tos::io::serial_backend_errors::out_of_memory:
            case tos::io::serial_backend_errors::stream_closed:
                return;
            default:
                continue;
            }
        }

        auto& [stream, p] = force_get(res);

        handler(stream, std::move(p));
    }
}

template<class SerialT>
class serial_channel : public any_channel {
public:
    serial_channel(serial_backend<SerialT>& backend, int stream)
        : m_backend{&backend}
        , m_stream_id{stream} {
    }

    void send(span<const uint8_t> span) override {
        m_backend->send(m_stream_id, span);
    }

    tos::intrusive_ptr<packet> receive() override {
        m_wait.down();
        return std::move(m_packet);
    }

    void receive(intrusive_ptr<packet> packet) {
        m_packet = std::move(packet);
        m_wait.up();
    }

private:
    serial_backend<SerialT>* m_backend;
    int m_stream_id;

    intrusive_ptr<packet> m_packet;
    semaphore m_wait{0};
};

template<class SerialT>
intrusive_ptr<serial_channel<SerialT>> make_channel(serial_backend<SerialT>& serial,
                                                    int stream_id) {
    return make_intrusive<serial_channel<SerialT>>(serial, stream_id);
}
} // namespace tos::io