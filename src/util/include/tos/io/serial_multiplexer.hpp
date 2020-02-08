#pragma once
#include <algorithm>
#include <optional>
#include <tos/crc32.hpp>
#include <tos/expected.hpp>
#include <tos/fixed_fifo.hpp>
#include <tos/self_pointing.hpp>
#include <vector>

namespace tos {

template<class MultiplexerT>
class multiplexed_stream : public self_pointing<multiplexed_stream<MultiplexerT>> {
public:
    using streamid_t = typename MultiplexerT::streamid_t;

    multiplexed_stream(MultiplexerT& multiplexer, streamid_t streamid)
        : m_multiplexer(&multiplexer)
        , m_streamid(streamid) {
    }

    int write(tos::span<const uint8_t> span) {
        return m_multiplexer->write(this->m_streamid, span);
    }

    tos::span<uint8_t> read(tos::span<uint8_t> span) {
        return m_multiplexer->read(this->m_streamid, span);
    }

private:
    streamid_t m_streamid;
    MultiplexerT* m_multiplexer;
};

enum class serial_multiplexer_errors
{
    stream_closed,
    bad_crc
};

template<size_t BufferSize>
struct stream_buffer {
    tos::basic_fixed_fifo<uint8_t, BufferSize, ring_buf> data;

    void append(tos::span<const uint8_t> span) {
        for (auto chr : span) {
            if (data.size() == data.capacity())
                return;
            data.push(chr);
        }
    }

    tos::span<uint8_t> read(tos::span<uint8_t> span) {
        for (auto& chr : span) {
            chr = data.pop();
        }
        return span;
    }

    void clear() {
        data = decltype(data){};
    }
};

template<class UsartT, class StreamDataT = stream_buffer<32>>
class serial_multiplexer {
public:
    using usart_type = UsartT;
    using streamid_t = uint16_t;
    using checksum_type = uint32_t;

    explicit serial_multiplexer(UsartT usart)
        : m_usart(std::move(usart)) {
    }

    multiplexed_stream<serial_multiplexer> create_stream(streamid_t streamid) {
        if (auto stream = find_stream(streamid); !stream) {
            m_streams.emplace_back(streamid, StreamDataT{});
        }
        return multiplexed_stream(*this, streamid);
    }

    std::optional<multiplexed_stream<serial_multiplexer>>
    get_stream(streamid_t streamid) {
        if (auto stream = find_stream(streamid); !stream) {
            return std::nullopt;
        }
        return multiplexed_stream(*this, streamid);
    }

    int write(streamid_t streamid, tos::span<const uint8_t> span) {
        this->m_usart->write(magic_numbers);
        this->m_usart->write(raw_cast<const uint8_t>(tos::monospan(streamid)));
        uint16_t size = span.size();
        this->m_usart->write(raw_cast<const uint8_t>(tos::monospan(size)));
        this->m_usart->write(span);
        uint32_t crc32 = tos::crc32(span);
        this->m_usart->write(raw_cast<const uint8_t>(tos::monospan(crc32)));

        return span.size();
    }

    tos::span<uint8_t> read(streamid_t streamid, tos::span<uint8_t> span) {
        auto stream = this->find_stream(streamid);

        if (!stream) {
            return tos::empty_span<uint8_t>();
        }

        auto readinto = span;

        while (true) {
            auto curslice =
                readinto.slice(0, std::min(readinto.size(), stream->data.size()));
            stream->read(curslice);
            readinto = readinto.slice(curslice.size());

            if (readinto.size() != 0) {
                while (true) {
                    auto next_packet_res = next_packet();
                    if (!next_packet_res) {
                        switch (force_error(next_packet_res)) {
                        case serial_multiplexer_errors::stream_closed:
                            // Stream is dead
                            return tos::empty_span<uint8_t>();
                        default:
                            continue;
                        }
                    }

                    // We got a packet
                    if (force_get(next_packet_res) == streamid) {
                        break;
                    }
                }
            } else {
                break;
            }
        }
        return span;
    }

    constexpr static std::array<uint8_t, 8> magic_numbers = {
        0x78, 0x9c, 0xc5, 0x45, 0xe3, 0xc8, 0x0e, 0x37};

private:
    StreamDataT* find_stream(streamid_t streamid) {
        auto it =
            std::find_if(this->m_streams.begin(),
                         this->m_streams.end(),
                         [streamid](auto& stream) { return stream.first == streamid; });
        if (it == this->m_streams.end())
            return nullptr;
        return &it->second;
    }

    tos::expected<streamid_t, serial_multiplexer_errors> next_packet() {
    begin_magicnumber:
        for (auto chr : magic_numbers) {
            uint8_t tmp;
            auto read_res = m_usart->read(tos::monospan(tmp));
            if (read_res.empty()) {
                return tos::unexpected(serial_multiplexer_errors::stream_closed);
            }
            if (tmp != chr) {
                goto begin_magicnumber;
            }
        }

        streamid_t streamid;
        auto read_res = m_usart->read(raw_cast<uint8_t>(tos::monospan(streamid)));
        if (read_res.empty()) {
            return tos::unexpected(serial_multiplexer_errors::stream_closed);
        }

        uint16_t size;
        read_res = m_usart->read(raw_cast<uint8_t>(tos::monospan(size)));
        if (read_res.empty()) {
            return tos::unexpected(serial_multiplexer_errors::stream_closed);
        }

        auto stream = find_stream(streamid);
        if (!stream) {
            // Received a packet for a non-existent stream, but we still have to read the
            // content + checksum
            for (uint16_t i = 0; i < size + sizeof(checksum_type); ++i) {
                uint8_t tmp;
                read_res = m_usart->read(tos::monospan(tmp));
                if (read_res.empty()) {
                    return tos::unexpected(serial_multiplexer_errors::stream_closed);
                }
            }
            return streamid;
        }

        uint32_t crc = 0;
        for (uint16_t i = 0; i < size; ++i) {
            uint8_t tmp;
            read_res = m_usart->read(tos::monospan(tmp));
            if (read_res.empty()) {
                // Did not get to read the whole packet, drop it
                stream->clear();
                return tos::unexpected(serial_multiplexer_errors::stream_closed);
            }
            stream->append(tos::monospan(tmp));
            crc = crc32(tos::raw_cast<const uint8_t>(tos::monospan(tmp)), crc);
        }

        uint32_t wire_crc;
        read_res = m_usart->read(tos::raw_cast<uint8_t>(tos::monospan(wire_crc)));
        if (read_res.empty()) {
            // Did not get to read the whole CRC, drop packet
            stream->clear();
            return tos::unexpected(serial_multiplexer_errors::stream_closed);
        }

        if (wire_crc != crc) {
            // CRC Mismatch, drop packet
            stream->clear();
            return tos::unexpected(serial_multiplexer_errors::bad_crc);
        }

        return streamid;
    }

    UsartT m_usart;
    std::vector<std::pair<streamid_t, StreamDataT>> m_streams;
};
} // namespace tos