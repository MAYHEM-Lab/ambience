#pragma once

#include <mailbox_generated.hpp>
#include <tos/mutex.hpp>

namespace tos::raspi3 {
class mailbox_channel {
public:
    explicit mailbox_channel(mailbox_channels channel)
        : m_channel(channel) {
    }

    uint32_t read();

    void write(uint32_t word);

private:
    static inline tos::mutex m_prot;
    mailbox_channels m_channel;
};

class property_channel_tags_builder {
public:
    property_channel_tags_builder() {
        push_word(0);
        push_word(0);
    }

    property_channel_tags_builder&
    add(uint32_t tag, span<const uint32_t> buffer, uint32_t response_len) {
        push_word(tag);
        push_word(std::max<uint32_t>(response_len, buffer.size_bytes()));
        push_word(0);
        push_words(buffer);
        return *this;
    }

    property_channel_tags_builder&
    add(uint32_t tag, std::initializer_list<uint32_t> buffer, uint32_t response_len) {
        return add(tag, span<const uint32_t>(buffer.begin(), buffer.end()), response_len);
    }

    property_channel_tags_builder& add(uint32_t tag,
                                       std::initializer_list<uint32_t> buffer) {
        return add(tag,
                   span<const uint32_t>(buffer.begin(), buffer.end()),
                   buffer.size() * sizeof(uint32_t));
    }

    span<uint32_t> end() {
        push_word(0);                               // End tag
        get_word(0) = size() * sizeof(get_word(0)); // Put hte size in bytes in 0th word
        return {m_buffer.get(), static_cast<size_t>(m_len)};
    }

private:
    void push_word(uint32_t word) {
        auto new_buf = std::make_unique<uint32_t[]>(m_len + 1);
        for (int i = 0; i < m_len; ++i) {
            new_buf[i] = m_buffer[i];
        }
        new_buf[m_len] = word;
        std::swap(new_buf, m_buffer);
        m_len++;
    }

    void push_words(span<const uint32_t> buffer) {
        for (auto w : buffer) {
            push_word(w);
        }
    }

    uint32_t& get_word(int i) {
        return m_buffer[i];
    }

    size_t size() const {
        return m_len;
    }

    int m_len = 0;
    std::unique_ptr<uint32_t[]> m_buffer;
};

class property_channel : mailbox_channel {
public:
    property_channel()
        : mailbox_channel(mailbox_channels::prop) {
    }

    [[nodiscard]] bool transaction(span<uint32_t> buffer);
};

class mailbox {
public:
private:
};
} // namespace tos::raspi3