#pragma once

#include "sink.hpp"
#include <memory>
#include <string_view>
#include <tos/mutex.hpp>
#include <tos/print.hpp>
#include <tos/self_pointing.hpp>

namespace tos::debug {
template<class SerialT>
class serial_sink
    : public detail::any_sink
    , public self_pointing<serial_sink<SerialT>> {
public:
    explicit serial_sink(SerialT ser, std::string_view tag = "")
        : m_serial(std::move(ser))
        , m_tag{tag} {
    }

    bool begin(log_level level) override {
        if (!platform::interrupts_disabled()) {
            m_prot->lock();
        }
        if (!m_tag.empty()) {
            tos::print(m_serial, "[", m_tag, "] ", tos::no_separator());
        }
        tos::print(m_serial, "[", level, "] [", seq++, "] ", tos::no_separator());
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

    void add(const void* ptr) override {
        tos::print(m_serial, ptr, "");
    }

    void add(log_level level) override {
        tos::print(m_serial, "[", level, "]", tos::no_separator());
        tos::print(m_serial, " ");
    }

    void add(span<const uint8_t> buf) override {
        tos::print(m_serial, buf, "");
    }

    void add(double d) override {
        tos::print(m_serial, d, "");
    }

    void end() override {
        tos::println(m_serial);
        if (!platform::interrupts_disabled()) {
            m_prot->unlock();
        }
    }

public:
    int seq = 0;
    std::unique_ptr<tos::mutex> m_prot = std::make_unique<tos::mutex>();
    SerialT m_serial;
    std::string_view m_tag;
};
} // namespace tos::debug