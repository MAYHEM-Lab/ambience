#pragma once

#include "sink.hpp"

#include <tos/print.hpp>
#include <tos/self_pointing.hpp>

namespace tos::debug {
template<class SerialT>
class serial_sink final
    : public detail::any_sink
    , public self_pointing<serial_sink<SerialT>> {
public:
    explicit serial_sink(SerialT ser)
        : m_serial(std::move(ser)) {
    }

    bool begin() override {
        tos::print(m_serial, "[serial_sink] ");
        return true;
    }

    void add(int64_t i) override {
        tos::print(m_serial, i, "");
    }

    void add(std::string_view str) override {
        tos::print(m_serial, str, "");
    }

    void end() override {
        tos::println(m_serial);
    }

public:
    SerialT m_serial;
};
} // namespace tos::debug