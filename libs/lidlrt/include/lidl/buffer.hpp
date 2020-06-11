#pragma once

#include <lidl/ptr.hpp>
#include <tos/span.hpp>

namespace lidl {
struct buffer {
public:
    explicit buffer(tos::span<uint8_t> buf)
        : m_buffer{buf} {
    }

    template<class T>
    T& operator[](ptr<T>& p) {
        auto off = p.get_offset();
        auto ptr_base = reinterpret_cast<uint8_t*>(&p);
        if (ptr_base - off < m_buffer.begin()) {
            // bad access!
        }
        return p.unsafe().get();
    }

    template<class T>
    const T& operator[](const ptr<T>& p) const {
        auto off = p.get_offset();
        auto ptr_base = reinterpret_cast<const uint8_t*>(&p);
        if (ptr_base - off < m_buffer.begin()) {
            // bad access!
        }
        return p.unsafe().get();
    }

    template<class T>
    const T& operator[](const ptr<const T>& p) const {
        auto off = p.get_offset();
        auto ptr_base = reinterpret_cast<const uint8_t*>(&p);
        if (ptr_base - off < m_buffer.begin()) {
            // bad access!
        }
        return p.unsafe().get();
    }

    template<class T>
    friend T& get_root(buffer buf);

    tos::span<const uint8_t> get_buffer() const {
        return m_buffer;
    }

private:
    tos::span<uint8_t> m_buffer;
};

template<class T>
T& get_root(buffer buf) {
    auto loc = buf.m_buffer.end() - sizeof(T);
    return *reinterpret_cast<T*>(loc);
}
} // namespace lidl