#pragma once

#include <lidlrt/ptr.hpp>
#include <tos/span.hpp>

namespace lidl {
template<class T, class BufferT>
auto& get_root(BufferT&& buf) {
    auto loc = &*(buf.end() - sizeof(T));
    return *const_cast<T*>(reinterpret_cast<const T*>(loc));
}
} // namespace lidl