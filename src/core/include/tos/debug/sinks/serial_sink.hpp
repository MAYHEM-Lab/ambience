#pragma once

#include "sink.hpp"

#include <tos/print.hpp>
#include <tos/self_pointing.hpp>
#include <tos/mutex.hpp>
#include <memory>

namespace tos::debug {
template<class SerialT>
class serial_sink
    : public detail::any_sink
    , public self_pointing<serial_sink<SerialT>> {
public:
    explicit serial_sink(SerialT ser)
        : m_serial(std::move(ser)) {
    }

    bool begin(log_level level) override {
        m_prot->lock();
        tos::print(m_serial, "[", level, "] ", tos::no_separator());
        return true;
    }

    void add(int64_t i) override {
        tos::print(m_serial, i, "");
    }

    void add(std::string_view str) override {
        tos::print(m_serial, str, "");
    }

    void add(bool b) override {
        tos::print(m_serial, b, "");
    }

    void add(void* ptr) override {
        tos::print(m_serial, ptr, "");
    }

    void add(log_level level) override {
        tos::print(m_serial, "[", level, "]", tos::no_separator());
        tos::print(m_serial, " ");
    }

    void add(span<const uint8_t> buf) override {
        tos::print(m_serial, buf, "");
    }

    void end() override {
        tos::println(m_serial);
        m_prot->unlock();
    }

public:
    std::unique_ptr<tos::mutex> m_prot = std::make_unique<tos::mutex>();
    SerialT m_serial;
};
} // namespace tos::debug