#pragma once

#include "detail/bcm2837.hpp"

#include <messagebox_generated.hpp>
#include <tos/mutex.hpp>

namespace tos::raspi3 {
class messagebox_channel {
public:
    explicit messagebox_channel(messagebox_channels channel) : m_channel(channel) {}

    uint32_t read();

    void write(uint32_t word);

private:
    static inline tos::mutex m_prot;
    messagebox_channels m_channel;
};

class property_channel_tags_builder {
public:
    property_channel_tags_builder() : m_buffer(2, 0) {}

    property_channel_tags_builder& add(uint32_t tag, span<const uint32_t> buffer, uint32_t response_len) {
        m_buffer.push_back(tag);
        m_buffer.push_back(std::max<uint32_t>(response_len, buffer.size_bytes()));
        m_buffer.push_back(0);
        m_buffer.insert(m_buffer.end(), buffer.begin(), buffer.end());
        return *this;
    }

    property_channel_tags_builder& add(uint32_t tag, std::initializer_list<uint32_t> buffer, uint32_t response_len) {
        return add(tag, span<const uint32_t>(buffer.begin(), buffer.end()), response_len);
    }

    property_channel_tags_builder& add(uint32_t tag, std::initializer_list<uint32_t> buffer) {
        return add(tag, span<const uint32_t>(buffer.begin(), buffer.end()), buffer.size() * sizeof(uint32_t));
    }

    span<uint32_t> end() {
        m_buffer.push_back(0);
        while ((m_buffer.size() / 4) % 16) {
            m_buffer.push_back(0);
        }
        m_buffer[0] = m_buffer.size() * 4;
        return m_buffer;
    }

private:
    std::vector<uint32_t> m_buffer;
};

class property_channel : messagebox_channel {
public:
    property_channel() : messagebox_channel(messagebox_channels::prop) {}

    [[nodiscard]]
    bool transaction(span<uint32_t> buffer);
};

class messagebox {
public:

private:
};
}