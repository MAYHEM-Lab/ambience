//
// Created by fatih on 8/22/18.
//

#pragma once

#include <cstdint>
#include <tos/expected.hpp>
#include <tos/self_pointing.hpp>
#include <tos/span.hpp>

namespace tos {
class omemory_stream : public self_pointing<omemory_stream> {
public:
    explicit constexpr omemory_stream(tos::span<uint8_t> buf)
        : m_buffer{buf}
        , m_wr_it{m_buffer.begin()} {
    }

    explicit omemory_stream(tos::span<char> buf)
        : omemory_stream(tos::raw_cast<uint8_t>(buf)) {
    }

    constexpr size_t write(tos::span<const uint8_t> buf) {
        auto buf_it = buf.begin();
        while (m_wr_it != m_buffer.end() && buf_it != buf.end()) {
            *m_wr_it++ = *buf_it++;
        }
        return buf_it - buf.begin();
    }

    constexpr tos::span<const uint8_t> get() {
        return m_buffer.slice(0, m_wr_it - m_buffer.begin());
    }

private:
    tos::span<uint8_t> m_buffer;
    tos::span<uint8_t>::iterator m_wr_it;
};

class imemory_stream : public self_pointing<imemory_stream> {
public:
    explicit constexpr imemory_stream(tos::span<const uint8_t> buf)
        : m_buffer{buf}
        , m_it{m_buffer.begin()} {
    }

    explicit imemory_stream(tos::span<const char> buf)
        : imemory_stream({reinterpret_cast<const uint8_t*>(buf.data()), buf.size()}) {
    }

    tos::expected<tos::span<uint8_t>, int> read(tos::span<uint8_t> buf) {
        auto len = std::min<ptrdiff_t>(buf.size(), std::distance(m_it, m_buffer.end()));
        // std::copy is not constexpr, thus we can't use it
        // this is std::copy(m_it, m_it + len, buf.begin())
        auto buf_it = buf.begin();
        for (int i = 0; i < len; ++i) {
            *buf_it++ = *m_it++;
        }
        return buf.slice(0, len);
    }

private:
    tos::span<const uint8_t> m_buffer;
    tos::span<const uint8_t>::iterator m_it;
};
} // namespace tos