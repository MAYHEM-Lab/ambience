//
// Created by fatih on 10/22/18.
//

#pragma once

#include <tos/span.hpp>

namespace tos {
/**
 * Reads from the given stream until the given bytes are matched at the end of the buffer.
 *
 * The `until` bytes will be at the end of the buffer.
 */
template<class CharT, class StreamT>
span<CharT> read_until(StreamT& str, span<const CharT> until, span<CharT> buffer) {
    for (size_t i = 0; i < buffer.size(); ++i) {
        auto res = str->read(tos::raw_cast<uint8_t>(buffer.slice(i, 1)));
        if (res.empty()) {
            return buffer.slice(0, i);
        }
        if (buffer.slice(i - until.size() + 1, until.size()) == until) {
            return buffer.slice(0, i + 1);
        }
    }
    return buffer;
}

template<class CharT, class StreamT>
span<CharT> read_to_end(StreamT& str, span<CharT> buf) {
    auto b = buf;
    while (b.size() != 0) {
        auto rd = str->read(b);
        if (!rd) {
            break;
        }

        auto& r = force_get(rd);
        b = b.slice(r.size(), b.size() - r.size());
    }
    return buf.slice(0, buf.size() - b.size());
}
} // namespace tos