#pragma once

#include <lidl/ptr.hpp>
#include <tos/span.hpp>

namespace lidl {
template<class T>
const T& get_root(tos::span<const uint8_t> buf) {
    auto loc = buf.end() - sizeof(T);
    return *reinterpret_cast<const T*>(loc);
}
} // namespace lidl