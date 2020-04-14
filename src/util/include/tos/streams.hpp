//
// Created by fatih on 10/22/18.
//

#pragma once

#include <tos/span.hpp>

namespace tos {
namespace detail {
template<class T>
struct read_returns_expected
    : std::bool_constant<!std::is_same_v<span<uint8_t>,
                                         decltype(std::declval<T>()->read(
                                             std::declval<span<uint8_t>>()))>> {};
} // namespace detail
/**
 * Reads from the given stream until the given bytes are matched at the end of the buffer.
 *
 * The `until` bytes will be at the end of the buffer.
 */
template<class CharT, class StreamT>
span<CharT> read_until(StreamT& str, span<const CharT> until, span<CharT> buffer) {
    for (size_t i = 0; i < buffer.size(); ++i) {
        auto raw_res = str->read(tos::raw_cast<uint8_t>(buffer.slice(i, 1)));

        span<uint8_t> res{nullptr};
        if constexpr (detail::read_returns_expected<StreamT>{}) {
            if (!raw_res) {
                return buffer.slice(0, i);
            }
            res = force_get(raw_res);
        } else {
            res = raw_res;
        }

        if (res.empty()) {
            return buffer.slice(0, i);
        }
        if (buffer.slice(i - until.size() + 1, until.size()) == until) {
            return buffer.slice(0, i + 1);
        }
    }
    return buffer;
}

/**
 * Reads from the given stream until the stream closes,
 * or the given buffer is filled up.
 *
 * A stream is considered closed if it returns an
 * unexpected value or a read reads 0 bytes.
 */
template<class CharT, class StreamT>
span<CharT> read_to_end(StreamT& str, span<CharT> buf) {
    auto b = buf;
    while (b.size() != 0) {
        auto rd = str->read(b);
        span<uint8_t> res{nullptr};

        if constexpr (detail::read_returns_expected<StreamT>{}) {
            if (!rd) {
                break;
            }
            res = force_get(rd);
        } else {
            res = rd;
        }

        if (res.empty()) {
            break;
        }

        b = b.slice(res.size(), b.size() - res.size());
    }
    return buf.slice(0, buf.size() - b.size());
}
} // namespace tos
