#pragma once

#include "sink.hpp"

#include <memory>
#include <tos/io/serial_packets.hpp>
#include <tos/mutex.hpp>
#include <tos/self_pointing.hpp>

namespace tos::debug {
enum class remote_sink_opcodes : uint8_t {
    begin = 'B',
    integer = 'i',
    string = 's',
    boolean = 'b',
    pointer = 'p',
    loglevel = 'l',
    data = 'd',
    end = 'E'
};

template<class SerialT>
class remote_sink
    : public detail::any_sink
    , public self_pointing<remote_sink<SerialT>> {
public:
    explicit remote_sink(io::serial_packets<SerialT>& ser)
        : m_serial(&ser) {
    }

    bool begin(log_level level) override {
        m_prot->lock();
        memcpy(buffer, "B", 1);
        memcpy(buffer + 1, &level, sizeof(level));
        m_serial->send(1, tos::span(buffer).slice(0, 1 + sizeof level));
        return true;
    }

    void add(int64_t i) override {
        memcpy(buffer, "i", 1);
        memcpy(buffer + 1, &i, sizeof i);
        m_serial->send(1, tos::span(buffer).slice(0, 1 + sizeof i));
    }

    void add(std::string_view str) override {
        memcpy(buffer, "s", 1);
        memcpy(buffer + 1, str.data(), str.size());
        m_serial->send(1, tos::span(buffer).slice(0, 1 + str.size()));
    }

    void add(bool b) override {
        memcpy(buffer, "b", 1);
        memcpy(buffer + 1, &b, sizeof b);
        m_serial->send(1, tos::span(buffer).slice(0, 1 + sizeof b));
    }

    void add(void* ptr) override {
        memcpy(buffer, "p", 1);
        memcpy(buffer + 1, &ptr, sizeof ptr);
        m_serial->send(1, tos::span(buffer).slice(0, 1 + sizeof ptr));
    }

    void add(log_level level) override {
        memcpy(buffer, "l", 1);
        memcpy(buffer + 1, &level, sizeof level);
        m_serial->send(1, tos::span(buffer).slice(0, 1 + sizeof level));
    }

    void add(span<const uint8_t> buf) override {
        memcpy(buffer, "d", 1);
        memcpy(buffer + 1, buf.data(), buf.size());
        m_serial->send(1, tos::span(buffer).slice(0, 1 + buf.size()));
    }

    void end() override {
        memcpy(buffer, "E", 1);
        m_serial->send(1, tos::span(buffer).slice(0, 1));
        m_prot->unlock();
    }

public:
    std::unique_ptr<tos::mutex> m_prot = std::make_unique<tos::mutex>();
    io::serial_packets<SerialT>* m_serial;
    uint8_t buffer[64];
};
} // namespace tos::debug