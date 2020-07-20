#pragma once

#include <algorithm>
#include <lidl/buffer.hpp>
#include <lidl/ptr.hpp>

namespace lidl {
struct message_builder {
public:
    message_builder(tos::span<uint8_t> buf)
        : m_buffer(buf)
        , m_cur_ptr(m_buffer.data()) {
    }

    uint16_t size() const {
        return static_cast<uint16_t>(m_cur_ptr - m_buffer.data());
    }

    tos::span<const uint8_t> get_buffer() const {
        auto whole = m_buffer;
        return whole.slice(0, size_t(size()));
    }

    uint8_t* allocate(size_t size, size_t align) {
        while ((this->size() % align) != 0) {
            m_cur_ptr++;
        }
        auto ptr = m_cur_ptr;
        m_cur_ptr += size;
        return ptr;
    }
private:
    tos::span<uint8_t> m_buffer;
    uint8_t* m_cur_ptr;
};

template<class T>
T& append_raw(message_builder& builder, const T& t) {
    auto alloc = builder.allocate(sizeof(T), alignof(T));
    auto ptr = new (alloc) T(t);
    return *ptr;
}

template<class T, class... Ts>
T& emplace_raw(message_builder& builder, Ts&&... args) {
    auto alloc = builder.allocate(sizeof(T), alignof(T));
    auto ptr = new (alloc) T{std::forward<Ts>(args)...};
    return *ptr;
}

template <class T, class... Ts>
T& create(message_builder& builder, Ts&&... args) {
    return emplace_raw<T>(builder, std::forward<Ts>(args)...);
}

template <class T>
void finish(message_builder& builder, T& t) {
    emplace_raw<ptr<T>>(builder, t);
}
} // namespace lidl