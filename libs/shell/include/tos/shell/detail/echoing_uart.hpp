//
// Created by fatih on 11/7/19.
//

#pragma once

#include <cstdint>
#include <tos/self_pointing.hpp>
#include <tos/span.hpp>

namespace tos {
namespace shell {
namespace detail {

template<class StreamT>
struct echoing_uart : tos::self_pointing<echoing_uart<StreamT>> {
    StreamT m_str;
    explicit echoing_uart(StreamT str)
        : m_str{std::move(str)} {
    }

    auto read(tos::span<uint8_t> buf) {
        auto ret = m_str->read(buf);
        m_str->write(ret);
        return ret;
    }

    auto write(tos::span<const uint8_t> buf) {
        return m_str->write(buf);
    }
};

} // namespace detail
} // namespace shell
} // namespace tos