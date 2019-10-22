#pragma once
#include <algorithm>
#include <optional>
#include <tos/crc32.hpp>
#include <tos/fixed_fifo.hpp>
#include <vector>

namespace tos {

template<class MultiplexerT>
class multiplexed_stream {
public:
    using streamid_t = typename MultiplexerT::streamid_t;

    multiplexed_stream(MultiplexerT& multiplexer, streamid_t streamid)
        : m_multiplexer(&multiplexer)
        , m_streamid(streamid) {
    }

    int write(tos::span<const char> span) {
        return m_multiplexer->write(this->m_streamid, span);
    }

    tos::span<char> read(tos::span<char> span) {
        return m_multiplexer->read(this->m_streamid, span);
    }

private:
    streamid_t m_streamid;
    MultiplexerT* m_multiplexer;
};

template<class UsartT, size_t BufferSize>
class serial_multiplexer {
public:
    using usart_type = UsartT;
    using streamid_t = uint16_t;

    explicit serial_multiplexer(UsartT usart)
        : m_usart(std::move(usart)) {
    }

    multiplexed_stream<serial_multiplexer> create_stream(streamid_t streamid) {
        if (auto stream = find_stream(streamid); !stream) {
            this->m_streams.emplace_back(streamid);
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

    int write(streamid_t streamid, tos::span<const char> span) {
        this->m_usart->write(magic_numbers);
        this->m_usart->write(raw_cast<const char>(tos::monospan(streamid)));
        uint16_t size = (uint64_t)span.size();
        this->m_usart->write(raw_cast<const char>(tos::monospan(size)));
        this->m_usart->write(span);
        uint32_t crc32 = tos::crc32(span);
        this->m_usart->write(raw_cast<const char>(tos::monospan(crc32)));

        return span.size();
    }

    tos::span<char> read(streamid_t streamid, tos::span<char> span) {
        auto stream = this->find_stream(streamid);

        if (!stream)
            return tos::empty_span<char>();

        auto readinto = span;

        while (true) {
            auto curslice =
                readinto.slice(0, std::min(readinto.size(), stream->data->size()));
            stream->read(curslice);
            readinto = readinto.slice(curslice.size());

            if (readinto.size() != 0) {
                while (next_packet() != streamid) {
                }
            } else
                break;
        }
        return span;
    }

    constexpr static std::array<char, 8> magic_numbers = {
        0x78, 0x9c, 0xc5, 0x45, 0xe3, 0xc8, 0x0e, 0x37};

private:
    struct stream_data {
        streamid_t streamid;
        std::unique_ptr<tos::fixed_fifo<char, BufferSize>> data;

        explicit stream_data(streamid_t streamid)
            : streamid(streamid)
            , data(std::make_unique<tos::fixed_fifo<char, BufferSize>>()) {
        }

        void append(tos::span<const char> span) {
            for (auto chr : span) {
                if (data->size() == data->capacity())
                    return;
                data->push(chr);
            }
        }

        tos::span<char> read(tos::span<char> span) {
            for (auto& chr : span) {
                chr = data->pop();
            }
            return span;
        }
    };

    stream_data* find_stream(streamid_t streamid) {
        auto it = std::find_if(
            this->m_streams.begin(), this->m_streams.end(), [streamid](auto& stream) {
                return stream.streamid == streamid;
            });
        if (it == this->m_streams.end())
            return nullptr;
        return &(*it);
    }

    streamid_t next_packet() {
    begin_magicnumber:
        for (auto chr : magic_numbers) {
            char tmp;
            m_usart->read(tos::monospan(tmp));
            if (tmp != chr) {
                goto begin_magicnumber;
            }
        }

        streamid_t streamid;
        uint16_t size;

        m_usart->read(raw_cast<char>(tos::monospan(streamid)));
        m_usart->read(raw_cast<char>(tos::monospan(size)));

        auto stream = find_stream(streamid);
        if (!stream) {
            for (uint16_t i = 0; i < size; ++i) {
                char tmp;
                m_usart->read(tos::monospan(tmp));
            }
            return streamid;
        }

        for (uint16_t i = 0; i < size; ++i) {
            char tmp;
            m_usart->read(tos::monospan(tmp));
            stream->append(tos::monospan(tmp));
        }

        return streamid;
    }

    UsartT m_usart;
    std::vector<stream_data> m_streams;
};

} // namespace tos