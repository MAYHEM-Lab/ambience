#pragma once

#include <algorithm>
#include <lidlrt/buffer.hpp>
#include <lidlrt/ptr.hpp>

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

    tos::span<uint8_t> get_buffer() {
        auto whole = m_buffer;
        return whole.slice(0, size_t(size()));
    }

    uint8_t* allocate(size_t size, size_t align) {
        auto tmp_cur_ptr = m_cur_ptr;
        while ((std::distance(m_buffer.begin(), tmp_cur_ptr) % align) != 0) {
            tmp_cur_ptr++;
        }

        if (std::distance(m_buffer.begin(), tmp_cur_ptr) + size >= m_buffer.size()) {
            return nullptr;
        }

        auto ptr = tmp_cur_ptr;
        m_cur_ptr = tmp_cur_ptr + size;
        return ptr;
    }

private:
    tos::span<uint8_t> m_buffer;
    uint8_t* m_cur_ptr;
};

template<class T>
T& append_raw(message_builder& builder, const T& t) {
    auto alloc = builder.allocate(sizeof(T), alignof(T));
    if (!alloc) {
        while (true)
            ;
    }
    auto ptr = new (alloc) T(t);
    return *ptr;
}

template<class T, class... Ts>
T& emplace_raw(message_builder& builder, Ts&&... args) {
    auto alloc = builder.allocate(sizeof(T), alignof(T));
    if (!alloc) {
        while (true)
            ;
    }
    auto ptr = new (alloc) T{std::forward<Ts>(args)...};
    return *ptr;
}

template<class T, class... Ts>
T& create(message_builder& builder, Ts&&... args) {
    return emplace_raw<T>(builder, std::forward<Ts>(args)...);
}

template<class T>
void finish(message_builder& builder, T& t) {
    emplace_raw<ptr<T>>(builder, t);
}
} // namespace lidl